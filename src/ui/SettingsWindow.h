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

    SettingsWindow(HINSTANCE instance, const Config& config, SaveFn onSave);

    // Shows the window, or brings an already open one to the front.
    void Show();
    void UpdateConfig(const Config& config);

    bool IsOpen() const { return hwnd_ != nullptr; }
    HWND Handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK RecorderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                         UINT_PTR id, DWORD_PTR ref);

    void CreateControls(HWND parent);
    void FillList();
    void UpdateListRow(int row);
    void ApplyRecordedShortcut();
    void ClearShortcut();
    void ReadControlsIntoConfig();
    void WriteConfigIntoControls();
    void ImportFromFile();
    void ExportToFile();
    int SelectedRow() const;

    HINSTANCE instance_;
    Config config_;      // working copy, only adopted on save
    SaveFn onSave_;

    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;
    HWND recorder_ = nullptr;
    HWND outerGap_ = nullptr;
    HWND innerGap_ = nullptr;
    HWND checkCycle_ = nullptr;
    HWND checkSnap_ = nullptr;
    HWND checkDisableAero_ = nullptr;
    HWND checkAutostart_ = nullptr;
    HWND checkCursor_ = nullptr;
    HWND checkUpdates_ = nullptr;
    HWND languageBox_ = nullptr;

    Shortcut recorded_;
};

}  // namespace wintangle
