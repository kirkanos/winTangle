#pragma once

#include <functional>

#include "config/Config.h"
#include "platform/Win32.h"

namespace wintangle {

// Einstellungsfenster: Tastenbelegung, Abstaende, Schalter, Import/Export.
//
// Bewusst von Hand aufgebaut statt als Dialogressource -- die Liste der
// Aktionen kommt aus dem Katalog, eine feste Ressourcendatei muesste bei jeder
// neuen Aktion nachgezogen werden.
class SettingsWindow {
public:
    // Wird beim Speichern aufgerufen. Die App uebernimmt die Konfiguration,
    // registriert die Hotkeys neu und schreibt die Datei.
    using SaveFn = std::function<void(const Config&)>;

    SettingsWindow(HINSTANCE instance, const Config& config, SaveFn onSave);

    // Zeigt das Fenster oder holt ein bereits offenes nach vorn.
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
    Config config_;      // Arbeitskopie, erst beim Speichern uebernommen
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

    Shortcut recorded_;
};

}  // namespace wintangle
