#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Merkt sich pro Fenster, was zuletzt passiert ist. Zwei Dinge hängen daran:
//
//  1. Restore  – der Rahmen, den das Fenster hatte, bevor WinTangle es das
//                erste Mal angefasst hat.
//  2. Zyklus   – wie oft dieselbe Aktion direkt hintereinander ausgeführt
//                wurde (linke Hälfte -> zwei Drittel -> ein Drittel).
//
// Beides wird ungültig, sobald das Fenster von außen bewegt oder in der Größe
// verändert wurde. Deshalb wird zusätzlich der Rahmen gespeichert, den wir
// zuletzt gesetzt haben: weicht der aktuelle davon ab, war jemand anderes dran.
class History {
public:
    // Eindeutige Fenster-ID (unter Windows der HWND als Zahl).
    using WindowId = std::uint64_t;

    struct Entry {
        Action lastAction = Action::Count_;
        int repeat = 0;        // Anzahl bisheriger identischer Ausführungen
        Rect appliedRect;      // was wir zuletzt gesetzt haben
        Rect restoreRect;      // Rahmen vor der ersten Aktion
        bool hasRestore = false;
    };

    // Wie oft `action` bereits direkt hintereinander auf dieses Fenster
    // angewandt wurde. 0, wenn die Kette unterbrochen ist.
    int RepeatCountFor(WindowId id, Action action, const Rect& currentRect) const;

    // Der Rahmen vor der ersten WinTangle-Aktion, falls bekannt.
    std::optional<Rect> RestoreRectFor(WindowId id) const;

    // Nach erfolgreicher Ausführung aufrufen. `before` ist der Rahmen vor der
    // Aktion, `after` der tatsächlich gesetzte.
    void Record(WindowId id, Action action, const Rect& before, const Rect& after);

    // Fenster wurde geschlossen bzw. Eintrag wird nicht mehr gebraucht.
    void Forget(WindowId id);

    size_t Size() const { return entries_.size(); }

private:
    // Toleranz, mit der ein gesetzter Rahmen als "unverändert" gilt. Manche
    // Apps korrigieren ihre Größe um ein paar Pixel (Mindestgröße, Rasterung),
    // das darf die Zykluskette nicht zerreißen.
    static constexpr int kTolerance = 8;

    static bool NearlyEqual(const Rect& a, const Rect& b);

    std::unordered_map<WindowId, Entry> entries_;
};

}  // namespace wintangle
