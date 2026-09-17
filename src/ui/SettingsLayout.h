#pragma once

#include "core/Rect.h"

// Positions of every control in the settings window, in client coordinates at
// 96 dpi.
//
// They live here as data rather than as numbers sprinkled through
// CreateControls() for one reason: a wrong coordinate is invisible to the
// compiler and to anyone who cannot open the window. As data, a test can check
// that no two controls overlap and that everything stays inside the client
// area -- which is how the language picker ended up sitting on top of a
// checkbox.
namespace wintangle::settings_layout {

// Client area. The window is sized from this with AdjustWindowRect, so these
// coordinates are exactly what the controls get.
inline constexpr int kClientWidth = 704;
inline constexpr int kClientHeight = 604;

// Smallest distance any control keeps from the window edge.
inline constexpr int kMargin = 10;

struct Slot {
    const char* name;  // only for test output
    Rect rect;
};

inline constexpr Slot kHeading{"heading", {12, 10, 412, 28}};
inline constexpr Slot kList{"list", {12, 32, 692, 362}};

// Assignment happens in the list itself, so all this row needs is the hint
// saying so, plus a discoverable alternative to the Delete key.
inline constexpr Slot kHint{"hint", {12, 375, 560, 393}};
inline constexpr Slot kClear{"clear", {574, 371, 692, 395}};

inline constexpr Slot kLabelOuterGap{"label:outer-gap", {12, 416, 142, 434}};
inline constexpr Slot kOuterGap{"outer-gap", {146, 413, 206, 435}};
inline constexpr Slot kLabelInnerGap{"label:inner-gap", {226, 416, 356, 434}};
inline constexpr Slot kInnerGap{"inner-gap", {360, 413, 420, 435}};

inline constexpr Slot kCheckCycle{"check:cycle", {12, 446, 332, 466}};
inline constexpr Slot kCheckSnap{"check:snap", {12, 470, 332, 490}};
inline constexpr Slot kCheckDisableAero{"check:disable-aero", {12, 494, 352, 514}};
inline constexpr Slot kCheckAutostart{"check:autostart", {360, 446, 680, 466}};
inline constexpr Slot kCheckCursor{"check:cursor", {360, 470, 680, 490}};
inline constexpr Slot kCheckUpdates{"check:updates", {360, 494, 680, 514}};

inline constexpr Slot kLabelLanguage{"label:language", {12, 526, 102, 544}};
// Height as it appears closed. The dropped-down list is taller; that extent is
// passed separately when the control is created.
inline constexpr Slot kLanguage{"language", {104, 522, 304, 546}};
inline constexpr int kLanguageDropdownHeight = 200;

inline constexpr Slot kImport{"import", {12, 566, 132, 592}};
inline constexpr Slot kExport{"export", {140, 566, 260, 592}};
inline constexpr Slot kRestoreDefaults{"restore-defaults", {268, 566, 428, 592}};
inline constexpr Slot kSave{"save", {472, 566, 572, 592}};
inline constexpr Slot kCancel{"cancel", {580, 566, 680, 592}};

// Every slot, for the overlap test.
inline constexpr Slot kAll[] = {
    kHeading, kList, kHint, kClear,
    kLabelOuterGap, kOuterGap, kLabelInnerGap, kInnerGap,
    kCheckCycle, kCheckSnap, kCheckDisableAero,
    kCheckAutostart, kCheckCursor, kCheckUpdates,
    kLabelLanguage, kLanguage,
    kImport, kExport, kRestoreDefaults, kSave, kCancel,
};

}  // namespace wintangle::settings_layout
