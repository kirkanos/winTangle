// WinTangle -- Fenstermanagement fuer Windows nach dem Vorbild von Rectangle.
//
// Aufbau: ein unsichtbares Nachrichtenfenster ist die Zentrale. Daran haengen
// die globalen Hotkeys (WM_HOTKEY), das Tray-Symbol, der Named-Pipe-Lauscher
// fuer wintangle://-Aufrufe und der Timer des Drag-Trackers. Alles laeuft im
// selben Thread; die einzige Ausnahme ist der Pipe-Thread, der nur eine
// Nachricht postet.

#include <memory>
#include <string>

#include "Executor.h"
#include "ScreenInfo.h"
#include "Win32.h"
#include "app/Autostart.h"
#include "app/Commands.h"
#include "app/HotkeyManager.h"
#include "app/Paths.h"
#include "app/TrayIcon.h"
#include "app/UriHandler.h"
#include "config/Config.h"
#include "core/Uri.h"
#include "snap/DragTracker.h"
#include "ui/SettingsWindow.h"

namespace wintangle {
namespace {

constexpr wchar_t kWindowClass[] = L"WinTangleMessageWindow";
constexpr wchar_t kMutexName[] = L"Local\\WinTangle.SingleInstance";

class App {
public:
    explicit App(HINSTANCE instance) : instance_(instance) {}

    bool Initialize();
    int Run();

    // Eine beim Start mitgegebene wintangle://-URI ausfuehren.
    void RunUri(const std::string& uri);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void LoadConfig();
    void SaveConfig();
    void ApplyConfig();          // Hotkeys, Snap-Tracker, AeroSnap-Schalter
    void RunAction(Action action);
    void ReportResult(Action action, ExecResult result);
    void HandleUriCommands();
    void ShowAbout();

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    UINT taskbarCreatedMsg_ = 0;

    Config config_ = Config::Defaults();
    std::unique_ptr<Executor> executor_;
    std::unique_ptr<HotkeyManager> hotkeys_;
    std::unique_ptr<TrayIcon> tray_;
    std::unique_ptr<DragTracker> dragTracker_;
    std::unique_ptr<SettingsWindow> settings_;
    UriServer uriServer_;

    // Damit die Meldung "Fenster gehoert einem Prozess mit hoeheren Rechten"
    // nicht bei jedem Tastendruck erscheint.
    bool accessDeniedReported_ = false;
};

void App::LoadConfig() {
    const std::wstring path = ConfigPath();
    if (path.empty()) return;

    std::string error;
    std::vector<std::string> warnings;
    Config loaded;
    if (!Config::LoadFromFile(Narrow(path), loaded, error, &warnings)) {
        MessageBoxW(nullptr,
                    (L"Die Konfiguration konnte nicht gelesen werden:\n" + Widen(error) +
                     L"\n\nEs gelten die Standardeinstellungen.")
                        .c_str(),
                    L"WinTangle", MB_ICONWARNING | MB_OK);
        return;
    }
    config_ = loaded;
}

void App::SaveConfig() {
    const std::wstring path = ConfigPath();
    if (path.empty()) return;
    std::string error;
    if (!config_.SaveToFile(Narrow(path), error) && tray_) {
        tray_->ShowBalloon(L"WinTangle", Widen(error), true);
    }
}

void App::ApplyConfig() {
    if (executor_) executor_->SetConfig(config_);
    if (dragTracker_) dragTracker_->SetConfig(config_);
    if (settings_) settings_->UpdateConfig(config_);

    const auto conflicts = hotkeys_->Apply(config_);
    if (!conflicts.empty() && tray_) {
        std::wstring text = L"Bereits von einem anderen Programm belegt:\n";
        for (const auto& c : conflicts) {
            text += L"• " + c.combo + L" (" + Widen(std::string(ActionLabel(c.action))) + L")\n";
        }
        tray_->ShowBalloon(L"Tastenkombinationen nicht verfügbar", text, true);
    }

    // Das Windows-eigene Andocken wuerde sonst mit unseren Snap-Bereichen
    // konkurrieren: beide reagieren auf dasselbe Ziehen an den Rand.
    // SPI_SETWINARRANGING nimmt den Wert direkt in pvParam, nicht als Zeiger.
    const UINT_PTR arranging = config_.disableWindowsSnap ? FALSE : TRUE;
    SystemParametersInfoW(SPI_SETWINARRANGING, 0, reinterpret_cast<void*>(arranging),
                          SPIF_SENDCHANGE);

    SetAutostartEnabled(config_.launchAtLogin);
}

bool App::Initialize() {
    LoadConfig();

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance_;
    wc.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&wc)) return false;

    // HWND_MESSAGE: unsichtbares Fenster, das nur Nachrichten bekommt.
    hwnd_ = CreateWindowExW(0, kWindowClass, L"WinTangle", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                            instance_, this);
    if (!hwnd_) return false;

    // Nach einem Absturz des Explorers muss das Tray-Symbol neu angemeldet werden.
    taskbarCreatedMsg_ = RegisterWindowMessageW(L"TaskbarCreated");

    executor_ = std::make_unique<Executor>(config_);
    hotkeys_ = std::make_unique<HotkeyManager>(hwnd_);

    HICON icon = static_cast<HICON>(
        LoadImageW(instance_, L"APPICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
    if (!icon) icon = LoadIconW(nullptr, IDI_APPLICATION);
    tray_ = std::make_unique<TrayIcon>(hwnd_, icon);

    dragTracker_ = std::make_unique<DragTracker>(
        hwnd_, instance_, config_, [this](HWND hwnd, Action action, const Rect& frame) {
            ReportResult(action, executor_->ApplyFrame(hwnd, action, frame));
        });
    dragTracker_->Start();

    settings_ = std::make_unique<SettingsWindow>(instance_, config_, [this](const Config& updated) {
        config_ = updated;
        ApplyConfig();
        SaveConfig();
    });

    uriServer_.Start(hwnd_);
    RegisterUriScheme();
    ApplyConfig();
    return true;
}

int App::Run() {
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        // Die Einstellungen sind ein normales Fenster: Tab und Eingabetaste
        // muessen dort funktionieren.
        if (settings_ && settings_->IsOpen() && IsDialogMessageW(settings_->Handle(), &msg)) {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK App::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* self = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    if (self) return self->HandleMessage(msg, wp, lp);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT App::HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == taskbarCreatedMsg_ && tray_) {
        tray_->Recreate();
        return 0;
    }

    switch (msg) {
        case WM_HOTKEY: {
            Action action{};
            if (hotkeys_->ActionForId(static_cast<int>(wp), action)) RunAction(action);
            return 0;
        }
        case WM_TIMER:
            if (dragTracker_) dragTracker_->OnTimer(static_cast<UINT_PTR>(wp));
            return 0;

        case kMsgTrayCallback:
            if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU) {
                tray_->ShowMenu(config_, IsAutostartEnabled());
            } else if (LOWORD(lp) == WM_LBUTTONDBLCLK) {
                settings_->Show();
            }
            return 0;

        case kMsgUriCommand:
            HandleUriCommands();
            return 0;

        case WM_COMMAND: {
            const unsigned id = LOWORD(wp);
            if (id >= kCmdActionBase) {
                const size_t index = id - kCmdActionBase;
                const auto& actions = AllActions();
                if (index < actions.size()) RunAction(actions[index]);
                return 0;
            }
            switch (id) {
                case kCmdSettings: settings_->Show(); return 0;
                case kCmdToggleSnapAreas:
                    config_.snapAreasEnabled = !config_.snapAreasEnabled;
                    ApplyConfig();
                    SaveConfig();
                    return 0;
                case kCmdToggleCycleSizes:
                    config_.cycleSizes = !config_.cycleSizes;
                    ApplyConfig();
                    SaveConfig();
                    return 0;
                case kCmdToggleAutostart:
                    config_.launchAtLogin = !IsAutostartEnabled();
                    ApplyConfig();
                    SaveConfig();
                    return 0;
                case kCmdAbout: ShowAbout(); return 0;
                case kCmdQuit: DestroyWindow(hwnd_); return 0;
                default: return 0;
            }
        }

        case WM_DESTROY:
            uriServer_.Stop();
            dragTracker_.reset();
            hotkeys_.reset();
            tray_.reset();
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }
    return DefWindowProcW(hwnd_, msg, wp, lp);
}

void App::RunUri(const std::string& uri) {
    const std::string name = ParseExecuteActionUri(uri);
    Action action{};
    if (!name.empty() && ActionFromName(name, action)) RunAction(action);
}

void App::RunAction(Action action) {
    ReportResult(action, executor_->Execute(action));
}

void App::ReportResult(Action action, ExecResult result) {
    if (result != ExecResult::AccessDenied) return;
    if (accessDeniedReported_ || !tray_) return;

    accessDeniedReported_ = true;
    tray_->ShowBalloon(
        L"Fenster nicht verschiebbar",
        L"Dieses Fenster gehört einem Prozess mit höheren Rechten. Damit WinTangle es "
        L"anordnen kann, muss WinTangle selbst als Administrator laufen.",
        true);
    (void)action;
}

void App::HandleUriCommands() {
    std::string uri;
    while (uriServer_.PopPending(uri)) {
        const std::string name = ParseExecuteActionUri(uri);
        Action action{};
        if (!name.empty() && ActionFromName(name, action)) {
            RunAction(action);
        } else if (tray_) {
            tray_->ShowBalloon(L"Unbekannter Aufruf", Widen(uri), true);
        }
    }
}

void App::ShowAbout() {
    MessageBoxW(nullptr,
                L"WinTangle\n\nFensteranordnung per Tastenkombination und Ziehen,\n"
                L"nach dem Vorbild von Rectangle für macOS.\n\n"
                L"Aktionen lassen sich auch per URL auslösen:\n"
                L"wintangle://execute-action?name=left-half",
                L"Über WinTangle", MB_ICONINFORMATION | MB_OK);
}

}  // namespace
}  // namespace wintangle

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    using namespace wintangle;

    // Zweite Instanz: eine eventuell mitgegebene wintangle://-URI an die
    // laufende Instanz weiterreichen und sich sofort beenden.
    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    const bool alreadyRunning = mutex && GetLastError() == ERROR_ALREADY_EXISTS;

    const std::string args = Narrow(commandLine ? commandLine : L"");
    if (alreadyRunning) {
        if (!ParseExecuteActionUri(args).empty()) {
            SendUriToRunningInstance(args);
        } else {
            MessageBoxW(nullptr, L"WinTangle läuft bereits (Symbol im Infobereich).", L"WinTangle",
                        MB_ICONINFORMATION | MB_OK);
        }
        if (mutex) CloseHandle(mutex);
        return 0;
    }

    App app(instance);
    if (!app.Initialize()) {
        MessageBoxW(nullptr, L"WinTangle konnte nicht gestartet werden.", L"WinTangle",
                    MB_ICONERROR | MB_OK);
        if (mutex) CloseHandle(mutex);
        return 1;
    }

    // Eine URI, die den Start ausgeloest hat, gleich ausfuehren.
    if (!args.empty()) app.RunUri(args);

    const int code = app.Run();
    if (mutex) CloseHandle(mutex);
    return code;
}
