# Reporting a false positive

Heuristic scanners flag `wintangle.exe`, AVG and Avast as `IDP.Generic`. This
file is the text to hand them, kept here because an unsigned release has to be
reported again for every new build — a report clears one file, not the project.

## Where

**Single file, quickest route** — the vendor's false positive form:

- AVG: <https://www.avg.com/en-ww/report-false-positive>
- Avast (same engine): see
  <https://businesshelp.avast.com/Content/Products/General_Help/SampleSubmission/SubmittingFalsePositives.htm>

Threat Labs are notified automatically, usually processed within 24 hours, and
clients receive the update within roughly another 24.

**Every future build** — the Avast/AVG Whitelisting Program:
<https://businesshelp.avast.com/Content/Products/General_Help/Whitelisting/WhitelistingProgram.htm>

That one registers the vendor rather than the file. Two requirements to know
about up front: registration needs an email address on a company domain — free
providers such as Gmail are rejected — and while a digital signature is not
mandatory, it is preferred and makes the case considerably easier.

## What to submit

The file itself, the detection name (`IDP.Generic`), the scanner version, and
the text below.

## Text

> WinTangle is an open source window manager for Windows, comparable to
> Rectangle for macOS. It arranges windows via global keyboard shortcuts and by
> dragging them to a screen edge.
>
> The executable is detected as IDP.Generic. The behaviour that presumably
> triggers this is the functionality of the program itself:
>
> - `RegisterHotKey` for global keyboard shortcuts,
> - `SetWinEventHook` (EVENT_SYSTEM_MOVESIZESTART/END, out-of-context) to
>   detect a window being dragged,
> - `EnumWindows`, `OpenProcess` with PROCESS_QUERY_LIMITED_INFORMATION and
>   `QueryFullProcessImageName` to find arrangeable windows and honour a
>   per-application ignore list,
> - `SetWindowPos` / `DeferWindowPos` to position those windows,
> - a Run key under HKEY_CURRENT_USER for optional autostart,
> - a named pipe for the wintangle:// URL protocol, used to trigger actions
>   from scripts.
>
> The program writes nothing outside HKEY_CURRENT_USER and the user profile,
> requires no administrator rights, contacts no network service except GitHub
> for its update check, and collects no data.
>
> Source code: https://github.com/kirkanos/winTangle
> The binary is built by GitHub Actions from that source; the build log is
> public and the release page carries SHA-256 checksums for every file.
>
> License: GPL-3.0-or-later.

## Checksums to quote

Take these from `SHA256SUMS.txt` on the release page being reported. For
0.1.0-rc5:

```
681a03bf2825bb4d6210d401bfd6bc83c9712fe0e986e38d0f6746223e0172fd  wintangle-0.1.0-rc5-setup.exe
4dad185298e98c7f02ac23373dddf02976f2033bce13e7f76583447d2e4143a5  wintangle-0.1.0-rc5-x64.zip
```

## Worth checking first

Look the hash up on VirusTotal. A single engine objecting while the rest do not
is what a false positive looks like, and that observation belongs in the report.
VirusTotal itself is not a submission channel — it only shows the verdicts.
