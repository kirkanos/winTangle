#pragma once

#include <functional>

#include "Footprint.h"
#include "config/Config.h"
#include "core/Action.h"
#include "platform/Win32.h"

namespace wintangle {

// Detects a window being dragged to a screen edge and shows the resulting
// position.
//
// Built on SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART/END) in
// WINEVENT_OUTOFCONTEXT mode, so everything runs inside our own process
// without injecting a DLL into foreign ones. While dragging, the cursor
// position is polled on a timer; a WH_MOUSE_LL hook would be more precise, but
// routing every mouse movement on the system through our process is not worth
// the difference.
class DragTracker {
public:
    // Called on release when the cursor was inside a snap zone.
    using ApplyFn = std::function<void(HWND hwnd, Action action, const Rect& frame)>;

    DragTracker(HWND host, HINSTANCE instance, const Config& config, ApplyFn apply);
    ~DragTracker();

    DragTracker(const DragTracker&) = delete;
    DragTracker& operator=(const DragTracker&) = delete;

    bool Start();
    void Stop();

    void SetConfig(const Config& config) { config_ = &config; }

    // From the host window procedure's WM_TIMER.
    void OnTimer(UINT_PTR timerId);

    static constexpr UINT_PTR kTimerId = 0x5754;  // "WT"

private:
    static void CALLBACK EventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject,
                                   LONG idChild, DWORD thread, DWORD time);
    void OnMoveSizeStart(HWND hwnd);
    void OnMoveSizeEnd(HWND hwnd);
    void UpdatePreview();

    HWND host_;
    HINSTANCE instance_;
    const Config* config_;
    ApplyFn apply_;

    HWINEVENTHOOK hook_ = nullptr;
    Footprint footprint_;
    HWND dragging_ = nullptr;
    Action pendingAction_ = Action::Count_;
    Rect pendingFrame_;
    bool hasPending_ = false;
};

}  // namespace wintangle
