# WinTangle

Fensteranordnung für Windows nach dem Vorbild von
[Rectangle](https://github.com/rxhanson/Rectangle) für macOS: Fenster per
globaler Tastenkombination oder per Ziehen an den Bildschirmrand auf Hälften,
Drittel, Viertel, Sechstel, Achtel und Neuntel setzen.

Natives C++/Win32, keine Runtime-Abhängigkeit, eine einzelne Exe.

## Stand

| Bereich | Stand |
|---|---|
| Berechnungskern (alle 58 Aktionen, Zyklus, Abstände, Layouts, Historie) | fertig, 48 Unit-Tests grün |
| Konfiguration (JSON, Shortcut-Parser, Import/Export) | fertig, getestet |
| URL-Schema `wintangle://` inkl. Parser | fertig, Parser getestet |
| Win32-Schicht (Fenster, Monitore, Hotkeys, Tray, Drag-Snap, Einstellungen) | geschrieben, **auf Windows noch nicht kompiliert/getestet** |

Entwickelt wird auf macOS, deshalb läuft dort nur der plattformfreie Teil.
Der Windows-Build passiert über die GitHub-Action `windows` oder lokal (siehe
unten) und ist der nächste offene Schritt.

## Bauen

Windows (Visual Studio 2022, C++-Desktop-Workload):

```
cmake -S . -B build -A x64
cmake --build build --config Release
build\src\platform\Release\wintangle.exe
```

macOS/Linux (nur Kern und Tests):

```
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

## Standardbelegung

`Win+Pfeil` gehört dem Windows-eigenen Andocken und lässt sich mit
`RegisterHotKey` nicht überschreiben — deshalb liegt alles auf `Ctrl+Alt`,
genau wie Rectangles `⌃⌥`.

| Aktion | Kombination |
|---|---|
| Linke / rechte / obere / untere Hälfte | `Ctrl+Alt+←/→/↑/↓` |
| Viertel | `Ctrl+Alt+U/I/J/K` |
| Drittel | `Ctrl+Alt+D/F/G`, zwei Drittel `Ctrl+Alt+E/T` |
| Maximieren | `Ctrl+Alt+Enter` |
| Höhe / Breite maximieren | `Ctrl+Alt+Shift+↑/↓` |
| Größer / kleiner | `Ctrl+Alt++` / `Ctrl+Alt+-` |
| Zentrieren / Wiederherstellen | `Ctrl+Alt+C` / `Ctrl+Alt+Backspace` |
| Nächster / vorheriger Bildschirm | `Ctrl+Alt+Win+→/←` |
| Alle kacheln / staffeln | `Ctrl+Alt+Shift+T` / `Ctrl+Alt+Shift+S` |

Sechstel, Achtel und Neuntel sind bewusst unbelegt und über das Tray-Menü,
den Einstellungsdialog oder die URL erreichbar.

Wiederholtes Drücken derselben Hälften-Kombination zykliert die Größe:
1/2 → 2/3 → 1/3.

## Konfiguration

`%APPDATA%\WinTangle\config.json`, im Einstellungsdialog bearbeitbar und
dort auch importier-/exportierbar. Zeilenkommentare (`//`) sind erlaubt.

```json
{
  "gaps": { "outer": 8, "inner": 8 },
  "cycleSizes": true,
  "snapAreasEnabled": true,
  "ignoredApps": ["vmware.exe"],
  "shortcuts": { "left-half": "Ctrl+Alt+Left" }
}
```

Ein vorhandener `shortcuts`-Block ersetzt die Standardbelegung vollständig;
ein leerer Wert (`""`) hebt eine Belegung ausdrücklich auf.

## Aktionen per URL

```
wintangle://execute-action?name=left-half
```

Gegenstück zu Rectangles `rectangle://`. Nützlich für Stream Deck, AutoHotkey
oder Skripte. Das Protokoll wird beim Start unter `HKCU` angemeldet.

## Aufbau

```
src/core/      Geometrie, Aktionskatalog, Layouts, Historie, JSON, URI,
               Snap-Zonen -- plattformfrei und vollständig unit-getestet
src/config/    Konfigurationsmodell, Laden/Speichern/Migrieren
src/platform/  Win32: Fensterzugriff, Monitore, Ausführung, WinMain
src/app/       Hotkeys, Tray-Symbol, Autostart, URL-Schema, Pfade
src/snap/      Drag-Erkennung und Vorschau-Overlay
src/ui/        Einstellungsfenster
```

Der Kern rechnet ausschließlich mit `Rect` und kennt kein `HWND`. Jede Aktion
ist eine reine Funktion `(Fenster, Arbeitsfläche, Wiederholung) → Rect`;
Hotkey, Tray-Menü, URL-Aufruf und Drag-Snap laufen alle durch denselben Pfad.

## Bekannte Grenzen

* **Fenster erhöhter Prozesse** (Task-Manager, als Administrator gestartete
  Programme) lassen sich nur anordnen, wenn WinTangle selbst erhöht läuft —
  das ist eine Windows-Sicherheitsgrenze (UIPI), keine Lücke im Programm.
  WinTangle meldet es einmalig statt still zu scheitern.
* **Virtuelle Desktops** werden nicht umgeschaltet. `IVirtualDesktopManager`
  ist offiziell nur lesend; alles andere hängt an undokumentierten
  COM-Schnittstellen, die Microsoft pro Windows-Build ändert. An derselben
  Grenze steht auch Rectangle Pro mit den macOS-Spaces.
* **Drag-Snap und Windows-AeroSnap** reagieren auf dasselbe Ziehen an den
  Rand. Im Einstellungsdialog lässt sich das Windows-eigene Andocken
  abschalten.

## Vorbild

Rectangle von Ryan Hanson, MIT-Lizenz. WinTangle ist eine eigenständige
Neuimplementierung für Win32 — übernommen sind Funktionsumfang, Standard-
belegung und das Verhalten, kein Code.
