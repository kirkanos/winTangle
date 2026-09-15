#pragma once

#include <functional>

#include "Win32.h"
#include "config/Config.h"
#include "core/Action.h"
#include "core/History.h"

namespace wintangle {

enum class ExecResult {
    Ok,
    NoTarget,        // no usable window in the foreground
    Ignored,         // the app is on the ignore list
    NothingToDo,     // restore without history, display switch with one display
    AccessDenied,    // window of an elevated process (UIPI)
    Failed,
};

// Runs actions. The single place where calculation, window access and history
// come together -- hotkey, tray menu, wintangle:// URI and drag-snap all call
// in here.
class Executor {
public:
    explicit Executor(const Config& config) : config_(&config) {}

    // Call after the settings have been saved.
    void SetConfig(const Config& config) { config_ = &config; }

    // `target` = nullptr means: the active window.
    ExecResult Execute(Action action, HWND target = nullptr);

    // Places a window on a given frame directly (drag-snap already knows the
    // target) and keeps the history up to date.
    ExecResult ApplyFrame(HWND hwnd, Action action, const Rect& frame);

    void ForgetWindow(HWND hwnd) { history_.Forget(reinterpret_cast<std::uint64_t>(hwnd)); }

    History& GetHistory() { return history_; }

private:
    ExecResult ExecuteMultiWindow(Action action, HWND reference);

    const Config* config_;
    History history_;
};

}  // namespace wintangle
