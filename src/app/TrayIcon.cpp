#include "TrayIcon.h"

#include <vector>

#include "Commands.h"
#include "core/Shortcut.h"

namespace wintangle {
namespace {

// Grouping the catalogue into submenus -- fifty entries in one list would be
// unusable.
struct Group {
    const wchar_t* label;
    Action first;
    Action last;  // inclusive
};

const std::vector<Group>& Groups() {
    static const std::vector<Group> kGroups{
        {L"Halves", Action::LeftHalf, Action::CenterHalf},
        {L"Quarters", Action::TopLeft, Action::BottomRight},
        {L"Thirds", Action::FirstThird, Action::LastTwoThirds},
        {L"Sixths", Action::TopLeftSixth, Action::BottomRightSixth},
        {L"Eighths", Action::TopLeftEighth, Action::BottomRightEighth},
        {L"Ninths", Action::TopLeftNinth, Action::BottomRightNinth},
        {L"Size", Action::Maximize, Action::Smaller},
        {L"Position", Action::Center, Action::MoveDown},
        {L"Displays", Action::NextDisplay, Action::PreviousDisplay},
        {L"Multiple windows", Action::TileAll, Action::CascadeActiveApp},
    };
    return kGroups;
}

std::wstring MenuLabel(Action a, const Config& config) {
    std::wstring label = Widen(std::string(ActionLabel(a)));
    if (const auto sc = config.ShortcutFor(a)) {
        label += L"\t" + Widen(FormatShortcut(*sc));
    }
    return label;
}

}  // namespace

TrayIcon::TrayIcon(HWND owner, HICON icon) : owner_(owner), icon_(icon) {
    data_.cbSize = sizeof(data_);
    data_.hWnd = owner_;
    data_.uID = 1;
    data_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data_.uCallbackMessage = kMsgTrayCallback;
    data_.hIcon = icon_;
    wcscpy_s(data_.szTip, L"WinTangle");
    Shell_NotifyIconW(NIM_ADD, &data_);
}

TrayIcon::~TrayIcon() { Shell_NotifyIconW(NIM_DELETE, &data_); }

void TrayIcon::Recreate() {
    Shell_NotifyIconW(NIM_DELETE, &data_);
    Shell_NotifyIconW(NIM_ADD, &data_);
}

void TrayIcon::ShowBalloon(const std::wstring& title, const std::wstring& text, bool warning) {
    NOTIFYICONDATAW balloon = data_;
    balloon.uFlags = NIF_INFO;
    balloon.dwInfoFlags = warning ? NIIF_WARNING : NIIF_INFO;
    wcsncpy_s(balloon.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(balloon.szInfo, text.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &balloon);
}

void TrayIcon::ShowMenu(const Config& config, bool autostartEnabled, bool updatesSupported) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    const auto& all = AllActions();
    auto indexOf = [&all](Action a) {
        for (size_t i = 0; i < all.size(); ++i) {
            if (all[i] == a) return i;
        }
        return size_t{0};
    };

    for (const Group& group : Groups()) {
        HMENU sub = CreatePopupMenu();
        for (size_t i = indexOf(group.first); i <= indexOf(group.last); ++i) {
            AppendMenuW(sub, MF_STRING, kCmdActionBase + i, MenuLabel(all[i], config).c_str());
        }
        AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(sub), group.label);
    }

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (config.snapAreasEnabled ? MF_CHECKED : 0), kCmdToggleSnapAreas,
                L"Snap Areas While Dragging");
    AppendMenuW(menu, MF_STRING | (config.cycleSizes ? MF_CHECKED : 0), kCmdToggleCycleSizes,
                L"Cycle Sizes");
    AppendMenuW(menu, MF_STRING | (autostartEnabled ? MF_CHECKED : 0), kCmdToggleAutostart,
                L"Launch at Login");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    if (updatesSupported) {
        AppendMenuW(menu, MF_STRING, kCmdCheckUpdates, L"Check for Updates…");
        AppendMenuW(menu, MF_STRING | (config.automaticUpdates ? MF_CHECKED : 0),
                    kCmdToggleAutoUpdates, L"Check for Updates Daily");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, kCmdSettings, L"Settings…");
    AppendMenuW(menu, MF_STRING, kCmdAbout, L"About WinTangle");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdQuit, L"Quit");

    POINT pt{};
    GetCursorPos(&pt);
    // Without SetForegroundWindow the menu stays open when clicking elsewhere.
    SetForegroundWindow(owner_);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, owner_, nullptr);
    PostMessageW(owner_, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

}  // namespace wintangle
