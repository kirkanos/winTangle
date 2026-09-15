#pragma once

#include <cstddef>
#include <vector>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Berechnet die Zielpositionen für die Mehr-Fenster-Aktionen (Kacheln,
// Zeilen, Spalten, Staffeln). Ebenfalls plattformfrei, damit das Layout ohne
// laufendes Windows testbar bleibt; die Fensterauswahl passiert im Win32-Teil.
//
// `count` ist die Anzahl der zu platzierenden Fenster, in Z-Order von vorne
// nach hinten. Das Ergebnis hat immer genau `count` Einträge.
std::vector<Rect> LayoutWindows(Action action, size_t count, const Rect& area, const Gaps& gaps);

}  // namespace wintangle
