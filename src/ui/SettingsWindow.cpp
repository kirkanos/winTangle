#include "SettingsWindow.h"

#include <commctrl.h>
#include <commdlg.h>

#include <algorithm>

#include <string>
#include <vector>

#include "app/Autostart.h"
#include "app/Paths.h"
#include "app/Updater.h"
#include "core/Shortcut.h"

namespace wintangle {
namespace {

constexpr wchar_t kClassName[] = L"WinTangleSettings";

enum : int {
    kIdList = 200,
    kIdRecorder,
    kIdAssign,
    kIdClear,
    kIdOuterGap,
    kIdInnerGap,
    kIdCheckCycle,
    kIdCheckSnap,
    kIdCheckDisableAero,
    kIdCheckAutostart,
    kIdCheckCursor,
    kIdCheckUpdates,
    kIdImport,
    kIdExport,
    kIdSave,
    kIdCancel,
};

HWND MakeControl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y,
                 int w, int h, int id, HINSTANCE instance) {
    HWND control = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, parent,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance,
                                   nullptr);
    // Without the system font, hand built windows look like Windows 95.
    SendMessageW(control, WM_SETFONT,
                 reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    return control;
}

void SetCheck(HWND control, bool checked) {
    SendMessageW(control, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool GetCheck(HWND control) {
    return SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

int GetInt(HWND edit) {
    wchar_t buffer[16]{};
    GetWindowTextW(edit, buffer, static_cast<int>(std::size(buffer)));
    return _wtoi(buffer);
}

void SetInt(HWND edit, int value) { SetWindowTextW(edit, std::to_wstring(value).c_str()); }

bool ChooseFile(HWND owner, bool save, std::wstring& path) {
    wchar_t buffer[MAX_PATH]{};
    if (!path.empty()) wcsncpy_s(buffer, path.c_str(), _TRUNCATE);

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"WinTangle configuration (*.json)\0*.json\0All files\0*.*\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = static_cast<DWORD>(std::size(buffer));
    ofn.lpstrDefExt = L"json";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);

    const BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    if (!ok) return false;
    path = buffer;
    return true;
}

}  // namespace

SettingsWindow::SettingsWindow(HINSTANCE instance, const Config& config, SaveFn onSave)
    : instance_(instance), config_(config), onSave_(std::move(onSave)) {}

void SettingsWindow::UpdateConfig(const Config& config) {
    config_ = config;
    if (hwnd_) {
        WriteConfigIntoControls();
        FillList();
    }
}

void SettingsWindow::Show() {
    if (hwnd_) {
        ShowWindow(hwnd_, SW_RESTORE);
        SetForegroundWindow(hwnd_);
        return;
    }

    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance_;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconW(instance_, L"APPICON");
    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(0, kClassName, L"WinTangle Settings",
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
                            CW_USEDEFAULT, 720, 620, nullptr, nullptr, instance_, this);
    if (!hwnd_) return;

    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* self = reinterpret_cast<SettingsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = static_cast<SettingsWindow*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
            self->CreateControls(hwnd);
            self->WriteConfigIntoControls();
            self->FillList();
            return 0;
        }
        case WM_COMMAND: {
            if (!self) break;
            switch (LOWORD(wp)) {
                case kIdAssign: self->ApplyRecordedShortcut(); return 0;
                case kIdClear: self->ClearShortcut(); return 0;
                case kIdImport: self->ImportFromFile(); return 0;
                case kIdExport: self->ExportToFile(); return 0;
                case kIdSave: {
                    self->ReadControlsIntoConfig();
                    if (self->onSave_) self->onSave_(self->config_);
                    DestroyWindow(hwnd);
                    return 0;
                }
                case kIdCancel:
                    DestroyWindow(hwnd);
                    return 0;
                default:
                    break;
            }
            break;
        }
        case WM_NOTIFY: {
            if (!self) break;
            auto* header = reinterpret_cast<NMHDR*>(lp);
            // Double clicking a row jumps straight into the capture field.
            if (header->idFrom == kIdList && header->code == NM_DBLCLK) {
                SetFocus(self->recorder_);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (self) self->hwnd_ = nullptr;
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// The capture field swallows every key press and displays it as a combination
// instead of entering text.
LRESULT CALLBACK SettingsWindow::RecorderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                              UINT_PTR id, DWORD_PTR ref) {
    auto* self = reinterpret_cast<SettingsWindow*>(ref);

    switch (msg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const unsigned vk = static_cast<unsigned>(wp);
            // Modifiers on their own are not a combination yet.
            const bool isModifier = vk == VK_CONTROL || vk == VK_MENU || vk == VK_SHIFT ||
                                    vk == VK_LWIN || vk == VK_RWIN;
            if (isModifier) return 0;

            Shortcut s;
            if (GetKeyState(VK_CONTROL) < 0) s.mods |= kModCtrl;
            if (GetKeyState(VK_MENU) < 0) s.mods |= kModAlt;
            if (GetKeyState(VK_SHIFT) < 0) s.mods |= kModShift;
            if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0) s.mods |= kModWin;
            s.vk = vk;

            if (s.mods == 0 || KeyName(s.vk).empty()) {
                SetWindowTextW(hwnd, L"Hold a modifier (Ctrl/Alt/Shift/Win)");
                self->recorded_ = Shortcut{};
                return 0;
            }
            self->recorded_ = s;
            SetWindowTextW(hwnd, Widen(FormatShortcut(s)).c_str());
            return 0;
        }
        case WM_CHAR:
        case WM_SYSCHAR:
            return 0;  // no text in the capture field
        case WM_NCDESTROY:
            RemoveWindowSubclass(hwnd, RecorderProc, id);
            break;
        default:
            break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void SettingsWindow::CreateControls(HWND parent) {
    MakeControl(parent, WC_STATICW, L"Actions and key combinations", 0, 12, 10, 400, 18, -1,
                instance_);

    list_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            12, 32, 680, 330, parent,
                            reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)), instance_,
                            nullptr);
    ListView_SetExtendedListViewStyle(list_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    col.cx = 420;
    col.pszText = const_cast<wchar_t*>(L"Action");
    ListView_InsertColumn(list_, 0, &col);
    col.cx = 240;
    col.pszText = const_cast<wchar_t*>(L"Key combination");
    ListView_InsertColumn(list_, 1, &col);

    MakeControl(parent, WC_STATICW, L"New combination:", 0, 12, 374, 120, 18, -1, instance_);
    recorder_ = MakeControl(parent, WC_EDITW, L"", WS_BORDER | ES_READONLY, 136, 371, 220, 24,
                            kIdRecorder, instance_);
    SetWindowSubclass(recorder_, RecorderProc, kIdRecorder, reinterpret_cast<DWORD_PTR>(this));

    MakeControl(parent, WC_BUTTONW, L"Assign", BS_PUSHBUTTON, 366, 371, 100, 24, kIdAssign,
                instance_);
    MakeControl(parent, WC_BUTTONW, L"Remove", BS_PUSHBUTTON, 474, 371, 100, 24, kIdClear,
                instance_);

    MakeControl(parent, WC_STATICW, L"Outer gap (px):", 0, 12, 416, 130, 18, -1, instance_);
    outerGap_ = MakeControl(parent, WC_EDITW, L"0", WS_BORDER | ES_NUMBER, 146, 413, 60, 22,
                            kIdOuterGap, instance_);
    MakeControl(parent, WC_STATICW, L"Inner gap (px):", 0, 226, 416, 130, 18, -1, instance_);
    innerGap_ = MakeControl(parent, WC_EDITW, L"0", WS_BORDER | ES_NUMBER, 360, 413, 60, 22,
                            kIdInnerGap, instance_);

    checkCycle_ = MakeControl(parent, WC_BUTTONW, L"Cycle sizes (1/2 → 2/3 → 1/3)",
                              BS_AUTOCHECKBOX, 12, 446, 320, 20, kIdCheckCycle, instance_);
    checkSnap_ = MakeControl(parent, WC_BUTTONW, L"Snap areas while dragging", BS_AUTOCHECKBOX, 12,
                             470, 320, 20, kIdCheckSnap, instance_);
    checkDisableAero_ =
        MakeControl(parent, WC_BUTTONW, L"Turn off Windows' own snapping", BS_AUTOCHECKBOX,
                    12, 494, 340, 20, kIdCheckDisableAero, instance_);
    checkAutostart_ = MakeControl(parent, WC_BUTTONW, L"Launch at login", BS_AUTOCHECKBOX, 360,
                                  446, 320, 20, kIdCheckAutostart, instance_);
    checkCursor_ = MakeControl(parent, WC_BUTTONW, L"Move cursor with window", BS_AUTOCHECKBOX, 360,
                               470, 320, 20, kIdCheckCursor, instance_);
    checkUpdates_ = MakeControl(parent, WC_BUTTONW, L"Check for updates daily",
                                BS_AUTOCHECKBOX, 360, 494, 320, 20, kIdCheckUpdates, instance_);
    // Without update checking compiled in the box stays visible but disabled,
    // so it is obvious that this build does not have it.
    EnableWindow(checkUpdates_, Updater::IsSupported());

    MakeControl(parent, WC_BUTTONW, L"Import…", BS_PUSHBUTTON, 12, 534, 120, 26, kIdImport,
                instance_);
    MakeControl(parent, WC_BUTTONW, L"Export…", BS_PUSHBUTTON, 140, 534, 120, 26, kIdExport,
                instance_);
    MakeControl(parent, WC_BUTTONW, L"Save", BS_DEFPUSHBUTTON, 472, 534, 100, 26, kIdSave,
                instance_);
    MakeControl(parent, WC_BUTTONW, L"Cancel", BS_PUSHBUTTON, 580, 534, 100, 26, kIdCancel,
                instance_);
}

void SettingsWindow::FillList() {
    if (!list_) return;
    ListView_DeleteAllItems(list_);

    const auto& actions = AllActions();
    for (size_t i = 0; i < actions.size(); ++i) {
        const std::wstring label = Widen(std::string(ActionLabel(actions[i])));
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = static_cast<int>(i);
        item.lParam = static_cast<LPARAM>(i);
        item.pszText = const_cast<wchar_t*>(label.c_str());
        ListView_InsertItem(list_, &item);
        UpdateListRow(static_cast<int>(i));
    }
    ListView_SetItemState(list_, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void SettingsWindow::UpdateListRow(int row) {
    const auto& actions = AllActions();
    if (row < 0 || static_cast<size_t>(row) >= actions.size()) return;

    const auto shortcut = config_.ShortcutFor(actions[static_cast<size_t>(row)]);
    const std::wstring text = shortcut ? Widen(FormatShortcut(*shortcut)) : L"—";
    ListView_SetItemText(list_, row, 1, const_cast<wchar_t*>(text.c_str()));
}

int SettingsWindow::SelectedRow() const {
    return list_ ? ListView_GetNextItem(list_, -1, LVNI_SELECTED) : -1;
}

void SettingsWindow::ApplyRecordedShortcut() {
    const int row = SelectedRow();
    if (row < 0 || !recorded_.IsValid()) return;

    const Action action = AllActions()[static_cast<size_t>(row)];

    // If the combination is already bound to another action, it is removed
    // there -- Windows could not tell two actions on one key apart anyway.
    std::vector<int> changed{row};
    for (auto it = config_.shortcuts.begin(); it != config_.shortcuts.end();) {
        if (it->second == recorded_ && it->first != action) {
            const auto& all = AllActions();
            for (size_t i = 0; i < all.size(); ++i) {
                if (all[i] == it->first) changed.push_back(static_cast<int>(i));
            }
            it = config_.shortcuts.erase(it);
        } else {
            ++it;
        }
    }

    config_.shortcuts[action] = recorded_;
    for (int r : changed) UpdateListRow(r);

    recorded_ = Shortcut{};
    SetWindowTextW(recorder_, L"");
}

void SettingsWindow::ClearShortcut() {
    const int row = SelectedRow();
    if (row < 0) return;
    config_.shortcuts.erase(AllActions()[static_cast<size_t>(row)]);
    UpdateListRow(row);
}

void SettingsWindow::WriteConfigIntoControls() {
    SetInt(outerGap_, config_.gaps.outer);
    SetInt(innerGap_, config_.gaps.inner);
    SetCheck(checkCycle_, config_.cycleSizes);
    SetCheck(checkSnap_, config_.snapAreasEnabled);
    SetCheck(checkDisableAero_, config_.disableWindowsSnap);
    SetCheck(checkAutostart_, IsAutostartEnabled());
    SetCheck(checkCursor_, config_.moveCursorWithWindow);
    SetCheck(checkUpdates_, config_.automaticUpdates);
}

void SettingsWindow::ReadControlsIntoConfig() {
    config_.gaps.outer = std::max(0, GetInt(outerGap_));
    config_.gaps.inner = std::max(0, GetInt(innerGap_));
    config_.cycleSizes = GetCheck(checkCycle_);
    config_.snapAreasEnabled = GetCheck(checkSnap_);
    config_.disableWindowsSnap = GetCheck(checkDisableAero_);
    config_.moveCursorWithWindow = GetCheck(checkCursor_);
    config_.automaticUpdates = GetCheck(checkUpdates_);
    config_.launchAtLogin = GetCheck(checkAutostart_);
}

void SettingsWindow::ImportFromFile() {
    std::wstring path;
    if (!ChooseFile(hwnd_, false, path)) return;

    Config imported;
    std::string error;
    std::vector<std::string> warnings;
    if (!Config::LoadFromFile(Narrow(path), imported, error, &warnings)) {
        MessageBoxW(hwnd_, Widen(error).c_str(), L"Import failed", MB_ICONERROR | MB_OK);
        return;
    }
    config_ = imported;
    WriteConfigIntoControls();
    FillList();

    if (!warnings.empty()) {
        std::string text = "Skipped:\n";
        for (const auto& w : warnings) text += "• " + w + "\n";
        MessageBoxW(hwnd_, Widen(text).c_str(), L"Import with warnings", MB_ICONWARNING | MB_OK);
    }
}

void SettingsWindow::ExportToFile() {
    std::wstring path = L"wintangle.json";
    if (!ChooseFile(hwnd_, true, path)) return;

    ReadControlsIntoConfig();
    std::string error;
    if (!config_.SaveToFile(Narrow(path), error)) {
        MessageBoxW(hwnd_, Widen(error).c_str(), L"Export failed", MB_ICONERROR | MB_OK);
    }
}

}  // namespace wintangle
