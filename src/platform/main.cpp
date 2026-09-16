// WinTangle -- window management for Windows, modelled on Rectangle for macOS.
// Copyright (C) 2026 Andreas Hacker
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version. It comes with absolutely no warranty; see the LICENSE file or
// <https://www.gnu.org/licenses/>.
//
// Structure: an invisible message window is the hub. Hanging off it are the
// global hotkeys (WM_HOTKEY), the tray icon, the named pipe listener for
// wintangle:// calls and the drag tracker's timer. Everything runs on the same
// thread; the sole exception is the pipe thread, which only posts a message.

#include <memory>
#include <string>
#include <string_view>

#include "Executor.h"
#include "ScreenInfo.h"
#include "Win32.h"
#include "app/Autostart.h"
#include "app/Cleanup.h"
#include "app/Commands.h"
#include "app/HotkeyManager.h"
#include "app/Paths.h"
#include "app/TrayIcon.h"
#include "app/WindowsSnap.h"
#include "app/Updater.h"
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

    // Run a wintangle:// URI that was passed in at startup.
    void RunUri(const std::string& uri);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void LoadConfig();
    void SaveConfig();
    void ApplyConfig();          // hotkeys, snap tracker, AeroSnap switch
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
    Updater updater_;

    // Keeps the "window belongs to a process with higher privileges" notice
    // from appearing on every single key press.
    bool accessDeniedReported_ = false;
    bool snapHookFailureReported_ = false;
    bool aeroSnapRepairFailed_ = false;
};

void App::LoadConfig() {
    // Before the config is read the language Windows uses is the best guess --
    // otherwise an error about an unreadable config would arrive in English on
    // a German system.
    SetLanguage(DetectUiLanguage());

    const std::wstring path = ConfigPath();
    if (path.empty()) return;

    std::string error;
    std::vector<std::string> warnings;
    Config loaded;
    if (!Config::LoadFromFile(Narrow(path), loaded, error, &warnings)) {
        MessageBoxW(nullptr,
                    (T(Str::MsgConfigUnreadable) + L"\n" + Widen(error) + L"\n\n" +
                     T(Str::MsgDefaultsApply))
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
    // The language first: everything below may already produce visible text.
    SetLanguage(config_.language.value_or(DetectUiLanguage()));

    if (executor_) executor_->SetConfig(config_);
    if (dragTracker_ && !dragTracker_->SyncWithConfig(config_) && tray_) {
        // Enabled but the hook was refused: say so once instead of leaving the
        // user to wonder why dragging does nothing.
        if (!snapHookFailureReported_) {
            snapHookFailureReported_ = true;
            tray_->ShowBalloon(T(Str::MsgSnapHookFailedTitle), T(Str::MsgSnapHookFailedText),
                               true);
        }
    }
    updater_.SetAutomaticChecks(config_.automaticUpdates);
    if (settings_) settings_->UpdateConfig(config_);

    const auto conflicts = hotkeys_->Apply(config_);
    if (!conflicts.empty() && tray_) {
        std::wstring text = T(Str::MsgHotkeyConflictIntro) + L"\n";
        for (const auto& c : conflicts) {
            text += L"• " + c.combo + L" (" + Widen(LocalizedActionLabel(c.action)) + L")\n";
        }
        tray_->ShowBalloon(T(Str::MsgHotkeyConflictTitle), text, true);
    }

    // Windows' own snapping competes with our snap areas -- both react to the
    // same drag towards an edge -- but it is the user's setting, so it is only
    // touched when they ask, and restored when they stop asking.
    ApplyWindowArrangingPreference(config_.disableWindowsSnap);

    SetAutostartEnabled(config_.launchAtLogin);
}

bool App::Initialize() {
    LoadConfig();

    // Earlier versions could leave Aero Snap switched off for users who never
    // asked. Undo that once; if it cannot be undone, say so after the tray icon
    // exists.
    aeroSnapRepairFailed_ = !RepairWindowArrangingIfDamagedByOldVersion(config_.disableWindowsSnap);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance_;
    wc.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&wc)) return false;

    // HWND_MESSAGE: an invisible window that only receives messages.
    hwnd_ = CreateWindowExW(0, kWindowClass, L"WinTangle", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                            instance_, this);
    if (!hwnd_) return false;

    // After Explorer crashes the tray icon has to be registered again.
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
    // The hook itself is installed by ApplyConfig() below, and only when snap
    // areas are enabled.

    settings_ = std::make_unique<SettingsWindow>(instance_, config_, [this](const Config& updated) {
        config_ = updated;
        ApplyConfig();
        SaveConfig();
    });

    // Only start WinSparkle once the configuration is in place -- the switch
    // for automatic checks goes straight to the library.
    updater_.Initialize(Widen(WINTANGLE_VERSION), config_.automaticUpdates);

    if (aeroSnapRepairFailed_ && tray_) {
        tray_->ShowBalloon(T(Str::MsgAeroSnapBrokenTitle), T(Str::MsgAeroSnapBrokenText), true);
    }

    uriServer_.Start(hwnd_);
    RegisterUriScheme();
    ApplyConfig();
    return true;
}

int App::Run() {
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        // The settings are an ordinary window: tab and enter have to work
        // inside it.
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
                tray_->ShowMenu(config_, IsAutostartEnabled(), Updater::IsSupported());
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
                case kCmdCheckUpdates: updater_.CheckWithUi(); return 0;
                case kCmdToggleAutoUpdates:
                    config_.automaticUpdates = !config_.automaticUpdates;
                    ApplyConfig();
                    SaveConfig();
                    return 0;
                case kCmdAbout: ShowAbout(); return 0;
                case kCmdQuit: DestroyWindow(hwnd_); return 0;
                default: return 0;
            }
        }

        case WM_DESTROY:
            updater_.Shutdown();
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
    tray_->ShowBalloon(T(Str::MsgWindowNotMovableTitle), T(Str::MsgWindowNotMovableText),
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
            tray_->ShowBalloon(T(Str::MsgUnknownCallTitle), Widen(uri), true);
        }
    }
}

void App::ShowAbout() {
    // GPL v3 section 5(d): an interactive program has to show the licence.
    const std::wstring text = L"WinTangle\n\n" + T(Str::AboutDescription) + L"\n\n" +
                              T(Str::AboutUrlHint) +
                              L"\nwintangle://execute-action?name=left-half\n\n" +
                              T(Str::AboutVersion) + L" " WINTANGLE_VERSION_W L"\n" +
                              T(Str::AboutCopyright) + L"\n" + T(Str::AboutLicense) +
                              // State worth seeing without a debugger when
                              // something does not work.
                              L"\n\n" + T(Str::AboutSnapAreas) + L" " +
                              T(dragTracker_ && dragTracker_->IsRunning()
                                    ? Str::AboutStateActive
                                    : Str::AboutStateInactive);
    MessageBoxW(nullptr, text.c_str(), T(Str::AboutTitle).c_str(),
                MB_ICONINFORMATION | MB_OK);
}

}  // namespace
}  // namespace wintangle

namespace wintangle {
namespace {

// "--cleanup" removes every trace and exits again. Called by the uninstaller,
// and available to portable users who have no setup. With "--silent" it runs
// without asking and without reporting.
int RunCleanup(bool silent, bool appRunning) {
    // Runs before any config is loaded, so follow Windows here.
    SetLanguage(DetectUiLanguage());

    if (appRunning && !silent) {
        MessageBoxW(nullptr, T(Str::CleanupStillRunning).c_str(), T(Str::CleanupTitle).c_str(),
                    MB_ICONWARNING | MB_OK);
        return 1;
    }
    if (!silent) {
        const int answer =
            MessageBoxW(nullptr, T(Str::CleanupConfirm).c_str(), T(Str::CleanupTitle).c_str(),
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
        if (answer != IDYES) return 0;
    }

    const CleanupReport report = RemoveAllTraces();
    if (!silent) {
        MessageBoxW(nullptr, FormatCleanupReport(report).c_str(), T(Str::CleanupDoneTitle).c_str(),
                    report.failed.empty() ? MB_ICONINFORMATION | MB_OK : MB_ICONWARNING | MB_OK);
    }
    return report.failed.empty() ? 0 : 1;
}

// Asks a running instance to quit and waits for it to actually be gone.
//
// The installer needs this: WinTangle has no ordinary window, only a
// message-only one and a tray icon, so Windows' Restart Manager -- which is
// what Inno Setup's CloseApplications relies on -- cannot see it. A running
// copy would keep wintangle.exe locked and the installer would fail to replace
// it with "MoveFile failed; code 5".
int QuitRunningInstance() {
    HWND target = FindWindowExW(HWND_MESSAGE, nullptr, kWindowClass, nullptr);
    if (!target) return 0;  // nothing running is a success, not an error

    PostMessageW(target, WM_CLOSE, 0, 0);

    // Wait for the window to disappear rather than guess at a sleep: the
    // installer runs right after this returns.
    for (int waited = 0; waited < 5000; waited += 50) {
        if (!IsWindow(target)) return 0;
        Sleep(50);
    }
    return 1;  // still there -- the installer will report the locked file
}

bool HasFlag(std::wstring_view args, std::wstring_view flag) {
    return args.find(flag) != std::wstring_view::npos;
}

}  // namespace
}  // namespace wintangle

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    using namespace wintangle;

    // Second instance: hand any wintangle:// URI over to the running instance
    // and exit right away.
    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    const bool alreadyRunning = mutex && GetLastError() == ERROR_ALREADY_EXISTS;

    const std::wstring wideArgs = commandLine ? commandLine : L"";
    const std::string args = Narrow(wideArgs);

    if (HasFlag(wideArgs, L"--quit")) {
        const int code = QuitRunningInstance();
        if (mutex) CloseHandle(mutex);
        return code;
    }

    if (HasFlag(wideArgs, L"--cleanup")) {
        const int code = RunCleanup(HasFlag(wideArgs, L"--silent"), alreadyRunning);
        if (mutex) CloseHandle(mutex);
        return code;
    }

    if (alreadyRunning) {
        if (!ParseExecuteActionUri(args).empty()) {
            SendUriToRunningInstance(args);
        } else {
            MessageBoxW(nullptr, T(Str::MsgAlreadyRunning).c_str(), L"WinTangle",
                        MB_ICONINFORMATION | MB_OK);
        }
        if (mutex) CloseHandle(mutex);
        return 0;
    }

    App app(instance);
    if (!app.Initialize()) {
        MessageBoxW(nullptr, T(Str::MsgStartFailed).c_str(), L"WinTangle",
                    MB_ICONERROR | MB_OK);
        if (mutex) CloseHandle(mutex);
        return 1;
    }

    // Run a URI that triggered the start straight away.
    if (!args.empty()) app.RunUri(args);

    const int code = app.Run();
    if (mutex) CloseHandle(mutex);
    return code;
}
