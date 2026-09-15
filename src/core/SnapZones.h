#pragma once

#include <optional>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

struct SnapZoneConfig {
    // Wie nah der Mauszeiger an den Bildschirmrand muss, in Pixeln bei 96 dpi.
    int edgeMargin = 16;
    // Anteil der Bildschirmhoehe, der an den Seitenraendern als Ecke zaehlt.
    double cornerFraction = 0.25;
};

// Welche Aktion loest ein Loslassen an dieser Mausposition aus?
//
// Aufteilung wie bei Rectangle:
//   linker/rechter Rand      -> Haelfte, in den Ecken das jeweilige Viertel
//   oberer Rand              -> maximieren
//   unterer Rand             -> Drittel, je nach waagerechter Position
//
// `bounds` ist der gesamte Monitor, nicht die Arbeitsflaeche: der Mauszeiger
// erreicht beim Ziehen auch den Bereich hinter der Taskleiste.
std::optional<Action> SnapZoneAt(int x, int y, const Rect& bounds, const SnapZoneConfig& cfg = {});

}  // namespace wintangle
