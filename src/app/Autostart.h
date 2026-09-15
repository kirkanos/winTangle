#pragma once

namespace wintangle {

// Autostart ueber HKCU\Software\Microsoft\Windows\CurrentVersion\Run.
// Bewusst der Run-Key und keine geplante Aufgabe: der Key braucht keine
// Administratorrechte. Laeuft WinTangle erhoeht (um Fenster erhoehter
// Prozesse anfassen zu koennen), muss der Autostart als geplante Aufgabe
// eingerichtet werden -- das meldet die App im Einstellungsdialog.
bool IsAutostartEnabled();
bool SetAutostartEnabled(bool enabled);

}  // namespace wintangle
