#include "SettingsWindow.h"

#include <commctrl.h>
#include <commdlg.h>

#include <algorithm>

#include <string>
#include <vector>

#include "app/Autostart.h"
#include "app/Paths.h"
#include "SettingsLayout.h"
#include "app/Updater.h"
#include "core/Shortcut.h"

namespace wintangle {
namespace {

constexpr wchar_t kClassName[] = L"WinTangleSettings";

enum : int {
    kIdList = 200,
    kIdClear,
    kIdOuterGap,
    kIdInnerGap,
    kIdCheckCycle,
    kIdCheckSnap,
    kIdCheckDisableAero,
    kIdCheckAutostart,
    kIdCheckCursor,
    kIdCheckUpdates,
    kIdLanguage,
    kIdLabelActions,
    kIdLabelHint,
    kIdLabelOuterGap,
    kIdLabelInnerGap,
    kIdLabelLanguage,
    kIdImport,
    kIdExport,
    kIdSave,
    kIdCancel,
};

HWND MakeControl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style,
                 const settings_layout::Slot& slot, int id, HINSTANCE instance) {
    const Rect& r = slot.rect;
    HWND control = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, r.left, r.top,
                                   r.Width(), r.Height(), parent,
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

// Fills the picker: "same as Windows" first, then one entry per language,
// each written in that language itself. The selection is preserved.
void FillLanguageBox(HWND box) {
    const auto selected = SendMessageW(box, CB_GETCURSEL, 0, 0);
    SendMessageW(box, CB_RESETCONTENT, 0, 0);
    SendMessageW(box, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(T(Str::SettingsLanguageAuto).c_str()));
    for (Language language : AllLanguages()) {
        const std::wstring name = Widen(LanguageDisplayName(language));
        SendMessageW(box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
    }
    if (selected != CB_ERR) SendMessageW(box, CB_SETCURSEL, static_cast<WPARAM>(selected), 0);
}

bool IsModifierKey(unsigned vk) {
    return vk == VK_CONTROL || vk == VK_MENU || vk == VK_SHIFT || vk == VK_LWIN || vk == VK_RWIN;
}

// The modifiers held right now, in the same bits RegisterHotKey uses.
unsigned CurrentModifiers() {
    unsigned mods = 0;
    if (GetKeyState(VK_CONTROL) < 0) mods |= kModCtrl;
    if (GetKeyState(VK_MENU) < 0) mods |= kModAlt;
    if (GetKeyState(VK_SHIFT) < 0) mods |= kModShift;
    if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0) mods |= kModWin;
    return mods;
}

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
    if (!hwnd_) return;

    SetLanguage(config_.language.value_or(DetectUiLanguage()));
    WriteConfigIntoControls();
    RelabelControls();
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

    // Size the window from the client area the layout was built for, instead
    // of guessing at borders and caption height.
    constexpr DWORD kStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT frame{0, 0, settings_layout::kClientWidth, settings_layout::kClientHeight};
    AdjustWindowRectEx(&frame, kStyle, FALSE, 0);

    hwnd_ = CreateWindowExW(0, kClassName, T(Str::SettingsTitle).c_str(), kStyle, CW_USEDEFAULT,
                            CW_USEDEFAULT, frame.right - frame.left, frame.bottom - frame.top,
                            nullptr, nullptr, instance_, this);
    if (!hwnd_) return;

    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
    // The list is where the work happens, so it starts with the focus.
    SetFocus(list_);
}

void SettingsWindow::OnLanguageChanged() {
    const auto selected = SendMessageW(languageBox_, CB_GETCURSEL, 0, 0);
    if (selected == CB_ERR) return;

    if (selected == 0) {
        SetLanguage(DetectUiLanguage());
    } else {
        const auto& all = AllLanguages();
        const size_t index = static_cast<size_t>(selected) - 1;
        if (index >= all.size()) return;
        SetLanguage(all[index]);
    }
    RelabelControls();
}

void SettingsWindow::RelabelControls() {
    if (!hwnd_) return;

    SetWindowTextW(hwnd_, T(Str::SettingsTitle).c_str());

    const struct {
        int id;
        Str text;
    } captions[] = {
        {kIdLabelActions, Str::SettingsActionsHeading},
        {kIdLabelHint, Str::SettingsAssignHint},
        {kIdLabelOuterGap, Str::SettingsOuterGap},
        {kIdLabelInnerGap, Str::SettingsInnerGap},
        {kIdLabelLanguage, Str::SettingsLanguage},
        {kIdClear, Str::SettingsRemove},
        {kIdCheckCycle, Str::SettingsCycleSizes},
        {kIdCheckSnap, Str::SettingsSnapAreas},
        {kIdCheckDisableAero, Str::SettingsDisableWindowsSnap},
        {kIdCheckAutostart, Str::SettingsLaunchAtLogin},
        {kIdCheckCursor, Str::SettingsMoveCursor},
        {kIdCheckUpdates, Str::SettingsCheckUpdates},
        {kIdImport, Str::SettingsImport},
        {kIdExport, Str::SettingsExport},
        {kIdSave, Str::SettingsSave},
        {kIdCancel, Str::SettingsCancel},
    };
    for (const auto& caption : captions) {
        if (HWND control = GetDlgItem(hwnd_, caption.id)) {
            SetWindowTextW(control, T(caption.text).c_str());
        }
    }

    // Column headers and the rows, which carry the action labels.
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT;
    std::wstring header = T(Str::SettingsColumnAction);
    col.pszText = header.data();
    ListView_SetColumn(list_, 0, &col);
    header = T(Str::SettingsColumnShortcut);
    col.pszText = header.data();
    ListView_SetColumn(list_, 1, &col);

    FillLanguageBox(languageBox_);

    // Keep the selected row across the rebuild, so the language switch does
    // not move the user somewhere else in a list of 58 entries.
    const int selectedRow = SelectedRow();
    FillList();
    if (selectedRow >= 0) {
        ListView_SetItemState(list_, selectedRow, LVIS_SELECTED | LVIS_FOCUSED,
                              LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(list_, selectedRow, FALSE);
    }
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* self = reinterpret_cast<SettingsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = static_cast<SettingsWindow*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
            self->languageAtOpen_ = CurrentLanguage();
            self->saved_ = false;
            self->CreateControls(hwnd);
            self->WriteConfigIntoControls();
            self->FillList();
            return 0;
        }
        case WM_COMMAND: {
            if (!self) break;
            if (LOWORD(wp) == kIdLanguage && HIWORD(wp) == CBN_SELCHANGE) {
                self->OnLanguageChanged();
                return 0;
            }
            switch (LOWORD(wp)) {
                case kIdClear: self->ClearShortcut(); return 0;
                case kIdImport: self->ImportFromFile(); return 0;
                case kIdExport: self->ExportToFile(); return 0;
                case kIdSave: {
                    self->ReadControlsIntoConfig();
                    self->saved_ = true;
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
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (self) {
                // Cancelled or closed: undo a language preview, otherwise the
                // tray menu would speak a language the user backed out of.
                if (!self->saved_) SetLanguage(self->languageAtOpen_);
                self->hwnd_ = nullptr;
            }
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Key capture on the action list itself: pick a row, press the combination.
//
// The dialog message loop is what makes this delicate. IsDialogMessage turns
// arrow keys into navigation and Enter into "press the default button" before
// any control sees them -- which is precisely why assigning Ctrl+Alt+Left or
// Ctrl+Alt+Enter did nothing at all. A control only gets those keys if it says
// so through WM_GETDLGCODE, and this one says so exactly while a modifier is
// held, so plain arrows still move through the list.
LRESULT CALLBACK SettingsWindow::ListProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id,
                                          DWORD_PTR ref) {
    auto* self = reinterpret_cast<SettingsWindow*>(ref);

    switch (msg) {
        case WM_GETDLGCODE:
            if (CurrentModifiers() != 0) return DLGC_WANTALLKEYS;
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const unsigned vk = static_cast<unsigned>(wp);
            if (IsModifierKey(vk)) break;  // a modifier alone is not a combination

            const unsigned mods = CurrentModifiers();
            if (mods == 0) {
                // Without a modifier the list keeps its normal behaviour, so
                // arrows and type-ahead still work. Delete clears the binding.
                if (vk == VK_DELETE || vk == VK_BACK) {
                    self->ClearShortcut();
                    return 0;
                }
                break;
            }

            if (!KeyName(vk).empty()) self->AssignShortcut(Shortcut{mods, vk});
            return 0;
        }

        case WM_SYSCHAR:
            // Swallows the beep Windows makes for Alt combinations.
            return 0;

        case WM_NCDESTROY:
            RemoveWindowSubclass(hwnd, ListProc, id);
            break;

        default:
            break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void SettingsWindow::CreateControls(HWND parent) {
    MakeControl(parent, WC_STATICW, T(Str::SettingsActionsHeading).c_str(), 0,
                settings_layout::kHeading, kIdLabelActions, instance_);

    const Rect& listRect = settings_layout::kList.rect;
    list_ = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            listRect.left, listRect.top, listRect.Width(), listRect.Height(),
                            parent,
                            reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)), instance_,
                            nullptr);
    ListView_SetExtendedListViewStyle(list_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    SetWindowSubclass(list_, ListProc, kIdList, reinterpret_cast<DWORD_PTR>(this));

    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    col.cx = 420;
    std::wstring columnAction = T(Str::SettingsColumnAction);
    col.pszText = columnAction.data();
    ListView_InsertColumn(list_, 0, &col);
    col.cx = 240;
    std::wstring columnShortcut = T(Str::SettingsColumnShortcut);
    col.pszText = columnShortcut.data();
    ListView_InsertColumn(list_, 1, &col);

    MakeControl(parent, WC_STATICW, T(Str::SettingsAssignHint).c_str(), 0,
                settings_layout::kHint, kIdLabelHint, instance_);
    MakeControl(parent, WC_BUTTONW, T(Str::SettingsRemove).c_str(), BS_PUSHBUTTON,
                settings_layout::kClear, kIdClear, instance_);

    MakeControl(parent, WC_STATICW, T(Str::SettingsOuterGap).c_str(), 0,
                settings_layout::kLabelOuterGap, kIdLabelOuterGap, instance_);
    outerGap_ = MakeControl(parent, WC_EDITW, L"0", WS_BORDER | ES_NUMBER,
                            settings_layout::kOuterGap, kIdOuterGap, instance_);
    MakeControl(parent, WC_STATICW, T(Str::SettingsInnerGap).c_str(), 0,
                settings_layout::kLabelInnerGap, kIdLabelInnerGap, instance_);
    innerGap_ = MakeControl(parent, WC_EDITW, L"0", WS_BORDER | ES_NUMBER,
                            settings_layout::kInnerGap, kIdInnerGap, instance_);

    checkCycle_ = MakeControl(parent, WC_BUTTONW, T(Str::SettingsCycleSizes).c_str(),
                              BS_AUTOCHECKBOX, settings_layout::kCheckCycle, kIdCheckCycle,
                              instance_);
    checkSnap_ = MakeControl(parent, WC_BUTTONW, T(Str::SettingsSnapAreas).c_str(),
                             BS_AUTOCHECKBOX, settings_layout::kCheckSnap, kIdCheckSnap,
                             instance_);
    checkDisableAero_ = MakeControl(parent, WC_BUTTONW,
                                    T(Str::SettingsDisableWindowsSnap).c_str(), BS_AUTOCHECKBOX,
                                    settings_layout::kCheckDisableAero, kIdCheckDisableAero,
                                    instance_);
    checkAutostart_ = MakeControl(parent, WC_BUTTONW, T(Str::SettingsLaunchAtLogin).c_str(),
                                  BS_AUTOCHECKBOX, settings_layout::kCheckAutostart,
                                  kIdCheckAutostart, instance_);
    checkCursor_ = MakeControl(parent, WC_BUTTONW, T(Str::SettingsMoveCursor).c_str(),
                               BS_AUTOCHECKBOX, settings_layout::kCheckCursor, kIdCheckCursor,
                               instance_);
    checkUpdates_ = MakeControl(parent, WC_BUTTONW, T(Str::SettingsCheckUpdates).c_str(),
                                BS_AUTOCHECKBOX, settings_layout::kCheckUpdates, kIdCheckUpdates,
                                instance_);
    // Without update checking compiled in the box stays visible but disabled,
    // so it is obvious that this build does not have it.
    EnableWindow(checkUpdates_, Updater::IsSupported());

    // Language picker. First entry follows Windows, then one entry per
    // language, each written in that language itself.
    MakeControl(parent, WC_STATICW, T(Str::SettingsLanguage).c_str(), 0,
                settings_layout::kLabelLanguage, kIdLabelLanguage, instance_);
    // The height passed here governs the dropped-down list, not the closed
    // control -- which is why the layout table records the closed height and
    // the dropdown extent separately.
    const Rect& languageRect = settings_layout::kLanguage.rect;
    languageBox_ = CreateWindowExW(0, WC_COMBOBOXW, L"",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST |
                                       WS_VSCROLL,
                                   languageRect.left, languageRect.top, languageRect.Width(),
                                   settings_layout::kLanguageDropdownHeight, parent,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdLanguage)),
                                   instance_, nullptr);
    SendMessageW(languageBox_, WM_SETFONT,
                 reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    FillLanguageBox(languageBox_);

    MakeControl(parent, WC_BUTTONW, T(Str::SettingsImport).c_str(), BS_PUSHBUTTON,
                settings_layout::kImport, kIdImport, instance_);
    MakeControl(parent, WC_BUTTONW, T(Str::SettingsExport).c_str(), BS_PUSHBUTTON,
                settings_layout::kExport, kIdExport, instance_);
    MakeControl(parent, WC_BUTTONW, T(Str::SettingsSave).c_str(), BS_DEFPUSHBUTTON,
                settings_layout::kSave, kIdSave, instance_);
    MakeControl(parent, WC_BUTTONW, T(Str::SettingsCancel).c_str(), BS_PUSHBUTTON,
                settings_layout::kCancel, kIdCancel, instance_);
}

void SettingsWindow::FillList() {
    if (!list_) return;
    ListView_DeleteAllItems(list_);

    const auto& actions = AllActions();
    for (size_t i = 0; i < actions.size(); ++i) {
        const std::wstring label = Widen(LocalizedActionLabel(actions[i]));
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
    const std::wstring text = shortcut ? Widen(FormatShortcut(*shortcut)) : T(Str::SettingsUnbound);
    ListView_SetItemText(list_, row, 1, const_cast<wchar_t*>(text.c_str()));
}

int SettingsWindow::SelectedRow() const {
    return list_ ? ListView_GetNextItem(list_, -1, LVNI_SELECTED) : -1;
}

void SettingsWindow::AssignShortcut(const Shortcut& shortcut) {
    const int row = SelectedRow();
    if (row < 0 || !shortcut.IsValid()) return;

    const Action action = AllActions()[static_cast<size_t>(row)];

    // If the combination is already bound to another action, it is removed
    // there -- Windows could not tell two actions on one key apart anyway.
    std::vector<int> changed{row};
    for (auto it = config_.shortcuts.begin(); it != config_.shortcuts.end();) {
        if (it->second == shortcut && it->first != action) {
            const auto& all = AllActions();
            for (size_t i = 0; i < all.size(); ++i) {
                if (all[i] == it->first) changed.push_back(static_cast<int>(i));
            }
            it = config_.shortcuts.erase(it);
        } else {
            ++it;
        }
    }

    config_.shortcuts[action] = shortcut;
    for (int r : changed) UpdateListRow(r);
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

    // Index 0 is "same as Windows", the rest follow AllLanguages().
    int languageIndex = 0;
    if (config_.language) {
        const auto& all = AllLanguages();
        for (size_t i = 0; i < all.size(); ++i) {
            if (all[i] == *config_.language) languageIndex = static_cast<int>(i) + 1;
        }
    }
    SendMessageW(languageBox_, CB_SETCURSEL, static_cast<WPARAM>(languageIndex), 0);
}

void SettingsWindow::ReadControlsIntoConfig() {
    config_.gaps.outer = std::max(0, GetInt(outerGap_));
    config_.gaps.inner = std::max(0, GetInt(innerGap_));
    config_.cycleSizes = GetCheck(checkCycle_);
    config_.snapAreasEnabled = GetCheck(checkSnap_);
    config_.disableWindowsSnap = GetCheck(checkDisableAero_);
    config_.moveCursorWithWindow = GetCheck(checkCursor_);
    config_.automaticUpdates = GetCheck(checkUpdates_);

    const auto selected = SendMessageW(languageBox_, CB_GETCURSEL, 0, 0);
    if (selected <= 0) {
        config_.language.reset();
    } else {
        const auto& all = AllLanguages();
        const size_t index = static_cast<size_t>(selected) - 1;
        if (index < all.size()) config_.language = all[index];
    }
    config_.launchAtLogin = GetCheck(checkAutostart_);
}

void SettingsWindow::ImportFromFile() {
    std::wstring path;
    if (!ChooseFile(hwnd_, false, path)) return;

    Config imported;
    std::string error;
    std::vector<std::string> warnings;
    if (!Config::LoadFromFile(Narrow(path), imported, error, &warnings)) {
        MessageBoxW(hwnd_, Widen(error).c_str(), T(Str::MsgImportFailed).c_str(),
                    MB_ICONERROR | MB_OK);
        return;
    }
    config_ = imported;
    WriteConfigIntoControls();
    FillList();

    if (!warnings.empty()) {
        std::wstring text = T(Str::MsgSkipped) + L"\n";
        for (const auto& w : warnings) text += L"• " + Widen(w) + L"\n";
        MessageBoxW(hwnd_, text.c_str(), T(Str::MsgImportWarnings).c_str(),
                    MB_ICONWARNING | MB_OK);
    }
}

void SettingsWindow::ExportToFile() {
    std::wstring path = L"wintangle.json";
    if (!ChooseFile(hwnd_, true, path)) return;

    ReadControlsIntoConfig();
    std::string error;
    if (!config_.SaveToFile(Narrow(path), error)) {
        MessageBoxW(hwnd_, Widen(error).c_str(), T(Str::MsgExportFailed).c_str(),
                    MB_ICONERROR | MB_OK);
    }
}

}  // namespace wintangle
