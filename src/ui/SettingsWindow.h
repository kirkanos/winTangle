#pragma once

#include <functional>

#include "config/Config.h"
#include "platform/Win32.h"

namespace wintangle {

// Settings window: key bindings, gaps, switches, import and export.
//
// Deliberately built by hand rather than as a dialog resource -- the list of
// actions comes from the catalogue, and a fixed resource file would have to be
// updated for every new action.
class SettingsWindow {
public:
    // Called on save. The app takes over the configuration, re-registers the
    // hotkeys and writes the file.
    using SaveFn = std::function<void(const Config&)>;

    // Called with true while the window is open. The global hotkeys have to be
    // given up for that time: RegisterHotKey takes a combination system wide,
    // so a registered Ctrl+Alt+Left never reaches this window at all -- it just
    // runs the action, on the settings window itself. Every combination worth
    // assigning is already taken, which made assignment impossible.
    using SuspendHotkeysFn = std::function<void(bool suspended)>;

    SettingsWindow(HINSTANCE instance, const Config& config, SaveFn onSave,
                   SuspendHotkeysFn onSuspendHotkeys);

    // Shows the window, or brings an already open one to the front.
    void Show();
    void UpdateConfig(const Config& config);

    bool IsOpen() const { return hwnd_ != nullptr; }
    HWND Handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    // Captures key combinations while the action list has focus, so a binding
    // is made where it is read. Also the reason the list has to claim the keys
    // from the dialog loop -- see WM_GETDLGCODE.
    static LRESULT CALLBACK ListProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id,
                                     DWORD_PTR ref);

    void CreateControls(HWND parent);

    // Re-applies every caption in the language currently in use. Called when
    // the language changes while the window is open -- through the picker or
    // by importing a config that names a different one.
    void RelabelControls();

    // Picker changed: switch language immediately so the effect is visible
    // before saving.
    void OnLanguageChanged();

    void FillList();
    void UpdateListRow(int row);
    void AssignShortcut(const Shortcut& shortcut);
    void ClearShortcut();
    void ReadControlsIntoConfig();
    void WriteConfigIntoControls();
    // Puts the working copy back to Config::Defaults(). Like every other
    // change in this window it only reaches disk on save.
    void RestoreDefaults();

    void ImportFromFile();
    void ExportToFile();
    int SelectedRow() const;

    HINSTANCE instance_;
    Config config_;      // working copy, only adopted on save
    SaveFn onSave_;
    SuspendHotkeysFn onSuspendHotkeys_;

    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;
    HWND outerGap_ = nullptr;
    HWND innerGap_ = nullptr;
    HWND checkCycle_ = nullptr;
    HWND checkSnap_ = nullptr;
    HWND checkDisableAero_ = nullptr;
    HWND checkAutostart_ = nullptr;
    HWND checkCursor_ = nullptr;
    HWND checkUpdates_ = nullptr;
    HWND languageBox_ = nullptr;

    // The language in use when the window opened. Cancelling has to undo a
    // live preview, otherwise the tray menu would keep a language the user
    // just backed out of.
    Language languageAtOpen_ = Language::English;
    bool saved_ = false;

};

}  // namespace wintangle
