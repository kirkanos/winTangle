#include "Updater.h"

#include "platform/Win32.h"

#if defined(WINTANGLE_WITH_WINSPARKLE)
#include <winsparkle.h>
#endif

namespace wintangle {
namespace {

#if defined(WINTANGLE_WITH_WINSPARKLE)
// Address of the appcast feed. Going through "latest" keeps the URL stable.
constexpr char kAppcastUrl[] =
    "https://github.com/kirkanos/winTangle/releases/latest/download/appcast.xml";

// WinSparkle remembers the time of the last check and any skipped versions
// under this key.
constexpr char kRegistryPath[] = "Software\\WinTangle\\Updates";

// Once a day is plenty for a tool of this size.
constexpr int kCheckIntervalSeconds = 24 * 60 * 60;

// WinSparkle asks these from its own thread, so the answer must not touch
// anything but this handle.
HWND g_window = nullptr;

// Nothing here is ever unsaved, so quitting for an update is always fine.
// Answering no would land the user in "WinTangle cannot be restarted".
int __cdecl CanShutdown() { return 1; }

// Called after the installer has been launched, never before -- WinSparkle
// runs the installer first and only then asks the application to leave. Posted
// rather than called, because this arrives on WinSparkle's thread.
void __cdecl RequestShutdown() {
    if (g_window) PostMessageW(g_window, WM_CLOSE, 0, 0);
}
#endif

}  // namespace

Updater::~Updater() { Shutdown(); }

bool Updater::IsSupported() {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    return true;
#else
    return false;
#endif
}

void Updater::Initialize([[maybe_unused]] HWND window,
                         [[maybe_unused]] const std::wstring& version,
                         [[maybe_unused]] bool automaticChecks) {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (initialized_) return;

    g_window = window;

    // Every set_* call has to happen before win_sparkle_init().
    win_sparkle_set_appcast_url(kAppcastUrl);
    win_sparkle_set_can_shutdown_callback(&CanShutdown);
    win_sparkle_set_shutdown_request_callback(&RequestShutdown);
    win_sparkle_set_registry_path(kRegistryPath);
    win_sparkle_set_app_details(L"WinTangle", L"WinTangle", version.c_str());
    win_sparkle_set_automatic_check_for_updates(automaticChecks ? 1 : 0);
    win_sparkle_set_update_check_interval(kCheckIntervalSeconds);
    win_sparkle_init();
    initialized_ = true;
#endif
}

void Updater::Shutdown() {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (!initialized_) return;
    win_sparkle_cleanup();
    g_window = nullptr;
    initialized_ = false;
#endif
}

void Updater::CheckWithUi() {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (!initialized_) return;
    win_sparkle_check_update_with_ui();
#else
    MessageBoxW(nullptr,
                (T(Str::MsgUpdatesDisabled) +
                 L"\nhttps://github.com/kirkanos/winTangle/releases")
                    .c_str(),
                L"WinTangle", MB_ICONINFORMATION | MB_OK);
#endif
}

void Updater::SetAutomaticChecks([[maybe_unused]] bool enabled) {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (!initialized_) return;
    win_sparkle_set_automatic_check_for_updates(enabled ? 1 : 0);
#endif
}

}  // namespace wintangle
