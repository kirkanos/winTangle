#!/usr/bin/env python3
"""Generates the appcast XML that WinSparkle reads for its update check.

The file is attached as an asset to every GitHub release. The program always
requests .../releases/latest/download/appcast.xml; GitHub redirects that to the
newest release, so the URL baked into the program never changes.

The changes from CHANGELOG.md are embedded as HTML rather than linked to the
release page, so WinSparkle can show them without a second network round trip.
"""

import argparse
import html
import os
import re
import sys
from email.utils import formatdate
from xml.sax.saxutils import escape


def changelog_section(path: str, version: str) -> list[str]:
    """Lines of the '## [version]' section up to the next heading."""
    if not os.path.exists(path):
        return []
    lines, collecting = [], False
    for line in open(path, encoding="utf-8"):
        line = line.rstrip("\n")
        if re.match(rf"^## \[{re.escape(version)}\]", line):
            collecting = True
            continue
        if collecting and (line.startswith("## ") or re.match(r"^\[[^\]]+\]: ", line)):
            break
        if collecting:
            lines.append(line)
    return lines


def inline(text: str) -> str:
    """Escapes text and turns `code` into <code> elements."""
    parts = re.split(r"`([^`]+)`", html.escape(text))
    # Odd indices are the contents of the backticks.
    return "".join(
        f"<code>{part}</code>" if i % 2 else part for i, part in enumerate(parts)
    )


def to_html(lines: list[str]) -> str:
    """The minimal Markdown subset that actually occurs in the changelog:
    third level headings, bullet lists, paragraphs."""
    out, in_list, paragraph = [], False, []

    def flush_paragraph():
        nonlocal paragraph
        if paragraph:
            out.append("<p>" + inline(" ".join(paragraph)) + "</p>")
            paragraph = []

    def close_list():
        nonlocal in_list
        if in_list:
            out.append("</ul>")
            in_list = False

    for line in lines:
        stripped = line.strip()
        if not stripped:
            flush_paragraph()
            close_list()
        elif stripped.startswith("### "):
            flush_paragraph()
            close_list()
            out.append("<h3>" + inline(stripped[4:]) + "</h3>")
        elif stripped.startswith("- "):
            flush_paragraph()
            if not in_list:
                out.append("<ul>")
                in_list = True
            out.append("<li>" + inline(stripped[2:]) + "</li>")
        elif in_list and line.startswith("  "):
            # Continuation line of a bullet point
            out[-1] = out[-1][: -len("</li>")] + " " + inline(stripped) + "</li>"
        else:
            close_list()
            paragraph.append(stripped)

    flush_paragraph()
    close_list()
    return "\n".join(out)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--version", required=True, help="version announced to WinSparkle")
    p.add_argument(
        "--changelog-version",
        help="version to look up in the changelog; defaults to --version. A "
        "pre-release (0.1.0-rc1) carries the notes of 0.1.0.",
    )
    p.add_argument("--tag", required=True)
    p.add_argument("--repo", required=True, help="e.g. kirkanos/winTangle")
    p.add_argument("--installer", required=True, help="path to the setup executable")
    p.add_argument("--changelog", default="CHANGELOG.md")
    p.add_argument("--output", default="dist/appcast.xml")
    args = p.parse_args()

    if not os.path.exists(args.installer):
        print(f"Installer not found: {args.installer}", file=sys.stderr)
        return 1

    size = os.path.getsize(args.installer)
    filename = os.path.basename(args.installer)
    url = f"https://github.com/{args.repo}/releases/download/{args.tag}/{filename}"

    notes = to_html(changelog_section(args.changelog, args.changelog_version or args.version))
    if not notes.strip():
        notes = f"<p>See https://github.com/{args.repo}/releases/tag/{args.tag}</p>"

    # Arguments handed to the setup when WinSparkle runs it. An update should
    # not walk the user through the wizard again: /SILENT shows progress only,
    # and /UPDATED=1 is our own switch telling the installer to start the
    # program again afterwards, which /SILENT would otherwise suppress.
    installer_arguments = "/SILENT /SUPPRESSMSGBOXES /NORESTART /UPDATED=1"

    xml = f"""<?xml version="1.0" encoding="utf-8"?>
<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle">
  <channel>
    <title>WinTangle</title>
    <link>https://github.com/{escape(args.repo)}</link>
    <description>Updates for WinTangle</description>
    <language>en</language>
    <item>
      <title>Version {escape(args.version)}</title>
      <pubDate>{formatdate(localtime=False, usegmt=True)}</pubDate>
      <description><![CDATA[
{notes}
]]></description>
      <enclosure url="{escape(url)}"
                 sparkle:version="{escape(args.version)}"
                 sparkle:shortVersionString="{escape(args.version)}"
                 sparkle:os="windows"
                 sparkle:installerArguments="{escape(installer_arguments)}"
                 length="{size}"
                 type="application/octet-stream" />
    </item>
  </channel>
</rss>
"""

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as f:
        f.write(xml)
    print(f"{args.output} written ({size} bytes installer, {len(notes)} chars of notes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
