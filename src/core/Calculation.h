#pragma once

#include <optional>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Alles, was eine Aktion zur Berechnung braucht. Bewusst ein reiner Wert-Typ:
// Hotkey, Tray-Menü, wintangle://-URI und Drag-Snap füllen dieselbe Struktur.
struct CalcInput {
    Action action = Action::LeftHalf;
    Rect window;    // aktueller Fensterrahmen (schattenkorrigiert)
    Rect workArea;  // Arbeitsfläche des Monitors, auf dem das Fenster liegt
    Gaps gaps;

    // Anzahl der unmittelbar vorangegangenen identischen Ausführungen auf
    // demselben Fenster. 0 = erste Ausführung.
    int repeat = 0;
    bool cycleSizes = true;

    // Für Action::Restore.
    std::optional<Rect> restoreRect;

    // Für Next-/PreviousDisplay: Arbeitsfläche des Zielmonitors.
    std::optional<Rect> targetArea;
};

// Berechnet die Zielposition. Gibt nullopt zurück, wenn die Aktion nichts zu
// tun hat (z.B. Restore ohne Historie, Monitorwechsel bei nur einem Monitor)
// oder wenn sie über den Multi-Window-Pfad läuft.
std::optional<Rect> Calculate(const CalcInput& in);

// Gibt den Bruchteil zurück, den eine Rasteraktion belegt, oder nullopt für
// Aktionen, die nicht rasterbasiert sind (Center, Larger, Move, ...).
// `repeat` steuert das Zyklusverhalten der Hälften-Aktionen.
std::optional<Fraction> FractionFor(Action a, int repeat, bool cycleSizes);

// Überträgt ein Rect proportional von einer Arbeitsfläche auf eine andere.
// Wird für den Monitorwechsel gebraucht, damit ein halbiertes Fenster auf dem
// Zielmonitor wieder eine Hälfte ist statt pixelgleich zu bleiben.
Rect MapRectToArea(const Rect& rect, const Rect& from, const Rect& to);

}  // namespace wintangle
