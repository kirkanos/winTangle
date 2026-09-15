#pragma once

#include <functional>

#include "Footprint.h"
#include "config/Config.h"
#include "core/Action.h"
#include "platform/Win32.h"

namespace wintangle {

// Erkennt das Ziehen eines Fensters an einen Bildschirmrand und zeigt die
// Zielposition an.
//
// Umgesetzt mit SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART/END) im Modus
// WINEVENT_OUTOFCONTEXT -- damit laeuft alles im eigenen Prozess, ohne eine
// DLL in fremde Prozesse zu injizieren. Waehrend des Ziehens wird die
// Mausposition per Timer abgefragt; ein WH_MOUSE_LL-Hook waere genauer, aber
// jeder Mausbewegung des Systems durch unseren Prozess zu schicken ist den
// Unterschied nicht wert.
class DragTracker {
public:
    // Wird beim Loslassen aufgerufen, wenn der Zeiger in einer Snap-Zone war.
    using ApplyFn = std::function<void(HWND hwnd, Action action, const Rect& frame)>;

    DragTracker(HWND host, HINSTANCE instance, const Config& config, ApplyFn apply);
    ~DragTracker();

    DragTracker(const DragTracker&) = delete;
    DragTracker& operator=(const DragTracker&) = delete;

    bool Start();
    void Stop();

    void SetConfig(const Config& config) { config_ = &config; }

    // Aus WM_TIMER der Host-Fensterprozedur.
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
