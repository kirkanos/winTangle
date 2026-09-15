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

void Updater::Initialize([[maybe_unused]] const std::wstring& version,
                         [[maybe_unused]] bool automaticChecks) {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (initialized_) return;

    // Every set_* call has to happen before win_sparkle_init().
    win_sparkle_set_appcast_url(kAppcastUrl);
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
    initialized_ = false;
#endif
}

void Updater::CheckWithUi() {
#if defined(WINTANGLE_WITH_WINSPARKLE)
    if (!initialized_) return;
    win_sparkle_check_update_with_ui();
#else
    MessageBoxW(nullptr,
                L"This build was made without update checking.\n"
                L"New versions are available at:\n"
                L"https://github.com/kirkanos/winTangle/releases",
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
