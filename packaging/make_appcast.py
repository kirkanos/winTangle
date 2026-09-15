#!/usr/bin/env python3
"""Erzeugt die Appcast-XML, die WinSparkle fuer die Update-Pruefung liest.

Die Datei haengt als Asset an jedem GitHub-Release. Das Programm fragt immer
.../releases/latest/download/appcast.xml ab; GitHub leitet auf das neueste
Release um, die URL im Programm bleibt damit fuer immer gleich.

Die Aenderungen aus der CHANGELOG.md werden als HTML eingebettet, statt auf
die Release-Seite zu verlinken -- so zeigt WinSparkle sie auch ohne zweiten
Netzzugriff an.
"""

import argparse
import html
import os
import re
import sys
from email.utils import formatdate
from xml.sax.saxutils import escape


def changelog_section(path: str, version: str) -> list[str]:
    """Zeilen des Abschnitts '## [version]' bis zur naechsten Ueberschrift."""
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
    """Escapt und wandelt `code` in <code>-Elemente."""
    parts = re.split(r"`([^`]+)`", html.escape(text))
    # Ungerade Indizes sind die Inhalte der Backticks.
    return "".join(
        f"<code>{part}</code>" if i % 2 else part for i, part in enumerate(parts)
    )


def to_html(lines: list[str]) -> str:
    """Minimale Markdown-Teilmenge, die in der CHANGELOG tatsaechlich vorkommt:
    Ueberschriften der dritten Ebene, Aufzaehlungen, Absaetze."""
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
            # Fortsetzungszeile eines Aufzaehlungspunktes
            out[-1] = out[-1][: -len("</li>")] + " " + inline(stripped) + "</li>"
        else:
            close_list()
            paragraph.append(stripped)

    flush_paragraph()
    close_list()
    return "\n".join(out)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--version", required=True)
    p.add_argument("--tag", required=True)
    p.add_argument("--repo", required=True, help="z.B. kirkanos/winTangle")
    p.add_argument("--installer", required=True, help="Pfad zur Setup-Exe")
    p.add_argument("--changelog", default="CHANGELOG.md")
    p.add_argument("--output", default="dist/appcast.xml")
    args = p.parse_args()

    if not os.path.exists(args.installer):
        print(f"Installer nicht gefunden: {args.installer}", file=sys.stderr)
        return 1

    size = os.path.getsize(args.installer)
    filename = os.path.basename(args.installer)
    url = f"https://github.com/{args.repo}/releases/download/{args.tag}/{filename}"

    notes = to_html(changelog_section(args.changelog, args.version))
    if not notes.strip():
        notes = f"<p>Siehe https://github.com/{args.repo}/releases/tag/{args.tag}</p>"

    xml = f"""<?xml version="1.0" encoding="utf-8"?>
<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle">
  <channel>
    <title>WinTangle</title>
    <link>https://github.com/{escape(args.repo)}</link>
    <description>Updates für WinTangle</description>
    <language>de</language>
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
                 length="{size}"
                 type="application/octet-stream" />
    </item>
  </channel>
</rss>
"""

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as f:
        f.write(xml)
    print(f"{args.output} geschrieben ({size} Bytes Installer, {len(notes)} Zeichen Notizen)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
