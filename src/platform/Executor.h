#pragma once

#include <functional>

#include "Win32.h"
#include "config/Config.h"
#include "core/Action.h"
#include "core/History.h"

namespace wintangle {

enum class ExecResult {
    Ok,
    NoTarget,        // kein brauchbares Fenster im Vordergrund
    Ignored,         // App steht auf der Ignorierliste
    NothingToDo,     // z.B. Restore ohne Historie, Monitorwechsel bei einem Monitor
    AccessDenied,    // Fenster eines erhoehten Prozesses (UIPI)
    Failed,
};

// Fuehrt Aktionen aus. Der einzige Ort, an dem Berechnung, Fensterzugriff und
// Historie zusammenkommen -- Hotkey, Tray-Menue, wintangle://-URI und
// Drag-Snap rufen alle hier hinein.
class Executor {
public:
    explicit Executor(const Config& config) : config_(&config) {}

    // Nach dem Speichern der Einstellungen aufrufen.
    void SetConfig(const Config& config) { config_ = &config; }

    // `target` = nullptr bedeutet: aktives Fenster.
    ExecResult Execute(Action action, HWND target = nullptr);

    // Setzt ein Fenster direkt auf einen Rahmen (Drag-Snap: das Ziel steht
    // schon fest) und pflegt dabei die Historie.
    ExecResult ApplyFrame(HWND hwnd, Action action, const Rect& frame);

    void ForgetWindow(HWND hwnd) { history_.Forget(reinterpret_cast<std::uint64_t>(hwnd)); }

    History& GetHistory() { return history_; }

private:
    ExecResult ExecuteMultiWindow(Action action, HWND reference);

    const Config* config_;
    History history_;
};

}  // namespace wintangle
