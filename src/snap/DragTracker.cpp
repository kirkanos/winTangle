#include "DragTracker.h"

#include "core/Calculation.h"
#include "core/SnapZones.h"
#include "platform/ScreenInfo.h"
#include "platform/WindowRef.h"

namespace wintangle {
namespace {

// WinEvent-Callbacks sind freie Funktionen ohne Nutzdaten. Da es genau einen
// Tracker gibt, reicht ein Zeiger auf die Instanz.
DragTracker* g_tracker = nullptr;

constexpr UINT kPollIntervalMs = 30;

int ScaleForDpi(int value, UINT dpi) {
    return MulDiv(value, static_cast<int>(dpi), 96);
}

}  // namespace

DragTracker::DragTracker(HWND host, HINSTANCE instance, const Config& config, ApplyFn apply)
    : host_(host), instance_(instance), config_(&config), apply_(std::move(apply)) {}

DragTracker::~DragTracker() { Stop(); }

bool DragTracker::Start() {
    if (hook_) return true;
    if (!footprint_.Create(instance_)) return false;

    g_tracker = this;
    hook_ = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZEEND, nullptr,
                            EventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    return hook_ != nullptr;
}

void DragTracker::Stop() {
    if (hook_) {
        UnhookWinEvent(hook_);
        hook_ = nullptr;
    }
    KillTimer(host_, kTimerId);
    footprint_.Hide();
    dragging_ = nullptr;
    hasPending_ = false;
    if (g_tracker == this) g_tracker = nullptr;
}

void CALLBACK DragTracker::EventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG idObject, LONG,
                                     DWORD, DWORD) {
    if (!g_tracker || idObject != OBJID_WINDOW || !hwnd) return;
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        g_tracker->OnMoveSizeStart(hwnd);
    } else if (event == EVENT_SYSTEM_MOVESIZEEND) {
        g_tracker->OnMoveSizeEnd(hwnd);
    }
}

void DragTracker::OnMoveSizeStart(HWND hwnd) {
    if (!config_->snapAreasEnabled) return;

    WindowRef w(hwnd);
    if (!w.IsManageable()) return;
    if (config_->IsIgnored(w.ExeName())) return;

    dragging_ = hwnd;
    hasPending_ = false;
    SetTimer(host_, kTimerId, kPollIntervalMs, nullptr);
}

void DragTracker::OnMoveSizeEnd(HWND hwnd) {
    KillTimer(host_, kTimerId);
    footprint_.Hide();

    const bool apply = hasPending_ && dragging_ == hwnd && config_->snapAreasEnabled;
    dragging_ = nullptr;
    hasPending_ = false;
    if (apply && apply_) apply_(hwnd, pendingAction_, pendingFrame_);
}

void DragTracker::OnTimer(UINT_PTR timerId) {
    if (timerId == kTimerId) UpdatePreview();
}

void DragTracker::UpdatePreview() {
    if (!dragging_ || !config_->snapAreasEnabled) {
        footprint_.Hide();
        return;
    }

    POINT cursor{};
    if (!GetCursorPos(&cursor)) return;

    const MonitorInfo monitor = MonitorForPoint(cursor);
    if (!monitor.IsValid()) return;

    SnapZoneConfig zoneCfg;
    zoneCfg.edgeMargin = ScaleForDpi(zoneCfg.edgeMargin, monitor.dpi);

    const auto action = SnapZoneAt(cursor.x, cursor.y, monitor.bounds, zoneCfg);
    if (!action) {
        footprint_.Hide();
        hasPending_ = false;
        return;
    }

    CalcInput in;
    in.action = *action;
    in.window = WindowRef(dragging_).Frame();
    in.workArea = monitor.work;
    in.gaps = config_->gaps;
    // Beim Ziehen nie zyklieren -- der Zeiger steht fuer genau eine Position.
    in.cycleSizes = false;
    in.repeat = 0;

    const auto frame = Calculate(in);
    if (!frame) {
        footprint_.Hide();
        hasPending_ = false;
        return;
    }

    pendingAction_ = *action;
    pendingFrame_ = *frame;
    hasPending_ = true;
    footprint_.ShowAt(*frame);
}

}  // namespace wintangle
