#include "Executor.h"

#include "ScreenInfo.h"
#include "WindowList.h"
#include "WindowRef.h"
#include "core/Calculation.h"
#include "core/Layout.h"

namespace wintangle {
namespace {

// Nach dem Setzen liest der Executor den tatsaechlichen Rahmen zurueck. Apps
// mit Mindest- oder Rastergroesse (Terminals, einige Electron-Apps) landen
// nicht exakt auf dem gewuenschten Wert; die Historie muss den echten Wert
// kennen, sonst reisst die Zykluskette beim naechsten Tastendruck.
Rect ApplyAndReadBack(WindowRef& w, const Rect& target, DWORD& error) {
    if (!w.SetFrame(target, &error)) return Rect{};
    return w.Frame();
}

}  // namespace

ExecResult Executor::ApplyFrame(HWND hwnd, Action action, const Rect& frame) {
    WindowRef w(hwnd);
    if (!w || !w.IsManageable()) return ExecResult::NoTarget;

    const Rect before = w.Frame();
    DWORD error = 0;
    const Rect actual = ApplyAndReadBack(w, frame, error);
    if (actual.IsEmpty()) {
        return error == ERROR_ACCESS_DENIED ? ExecResult::AccessDenied : ExecResult::Failed;
    }
    history_.Record(w.Id(), action, before, actual);
    return ExecResult::Ok;
}

ExecResult Executor::Execute(Action action, HWND target) {
    HWND hwnd = target ? target : GetForegroundWindow();
    if (!hwnd) return ExecResult::NoTarget;

    if (IsMultiWindowAction(action)) return ExecuteMultiWindow(action, hwnd);

    WindowRef w(hwnd);
    if (!w.IsManageable()) return ExecResult::NoTarget;
    if (config_->IsIgnored(w.ExeName())) return ExecResult::Ignored;

    const MonitorInfo monitor = MonitorForWindow(hwnd);
    if (!monitor.IsValid()) return ExecResult::NoTarget;

    CalcInput in;
    in.action = action;
    in.window = w.Frame();
    in.workArea = monitor.work;
    in.gaps = config_->gaps;
    in.cycleSizes = config_->cycleSizes;
    in.repeat = history_.RepeatCountFor(w.Id(), action, in.window);
    in.restoreRect = history_.RestoreRectFor(w.Id());

    if (action == Action::NextDisplay || action == Action::PreviousDisplay) {
        const int step = action == Action::NextDisplay ? +1 : -1;
        const auto neighbor = NeighborMonitor(monitor.handle, step);
        if (!neighbor) return ExecResult::NothingToDo;
        in.targetArea = neighbor->work;
    }

    const auto targetRect = Calculate(in);
    if (!targetRect) return ExecResult::NothingToDo;

    const ExecResult result = ApplyFrame(hwnd, action, *targetRect);

    if (result == ExecResult::Ok && config_->moveCursorWithWindow) {
        const Rect placed = WindowRef(hwnd).Frame();
        SetCursorPos(placed.CenterX(), placed.CenterY());
    }
    return result;
}

ExecResult Executor::ExecuteMultiWindow(Action action, HWND reference) {
    const MonitorInfo monitor = MonitorForWindow(reference);
    if (!monitor.IsValid()) return ExecResult::NoTarget;

    WindowFilter filter;
    filter.monitor = monitor.handle;
    if (action == Action::CascadeActiveApp) filter.processId = ProcessIdOf(reference);

    auto windows = ListWindows(filter);
    // Ignorierte Apps bleiben auch beim Kacheln unangetastet.
    std::erase_if(windows, [&](const WindowRef& w) { return config_->IsIgnored(w.ExeName()); });
    if (windows.empty()) return ExecResult::NoTarget;

    const auto targets = LayoutWindows(action, windows.size(), monitor.work, config_->gaps);
    if (targets.empty()) return ExecResult::NothingToDo;

    // Beim Staffeln soll das aktive Fenster oben liegen, also von hinten nach
    // vorne platzieren.
    const bool cascade = action == Action::CascadeAll || action == Action::CascadeActiveApp;

    // Rahmen vor dem Eingriff merken -- daran haengt spaeter "Wiederherstellen".
    std::vector<Rect> before(windows.size());
    for (size_t i = 0; i < windows.size(); ++i) before[i] = windows[i].Frame();

    // Maximierte Fenster muessen vor dem Batch wiederhergestellt werden;
    // ShowWindow laesst sich nicht in DeferWindowPos buendeln.
    for (auto& w : windows) w.EnsureRestored();

    HDWP batch = BeginDeferWindowPos(static_cast<int>(windows.size()));

    for (size_t i = 0; i < windows.size(); ++i) {
        const size_t index = cascade ? windows.size() - 1 - i : i;
        const RECT r = windows[index].ToWindowRect(targets[index]);
        if (batch) {
            batch = DeferWindowPos(batch, windows[index].Handle(), HWND_TOP, r.left, r.top,
                                   r.right - r.left, r.bottom - r.top,
                                   SWP_NOACTIVATE | (cascade ? 0 : SWP_NOZORDER));
        }
        if (!batch) {
            // Faellt der Batch aus (z.B. ein Fenster verschwindet mittendrin),
            // wird einzeln weitergemacht statt alles abzubrechen.
            DWORD error = 0;
            windows[index].SetFrame(targets[index], &error);
        }
    }
    if (batch) EndDeferWindowPos(batch);

    for (size_t i = 0; i < windows.size(); ++i) {
        history_.Record(windows[i].Id(), action, before[i], windows[i].Frame());
    }
    return ExecResult::Ok;
}

}  // namespace wintangle
