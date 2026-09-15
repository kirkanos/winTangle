#pragma once

#include <string>

namespace wintangle {

// Update checking through WinSparkle -- the Windows counterpart to Sparkle,
// which Rectangle uses. WinSparkle fetches an appcast XML, compares versions,
// shows the changes and launches the installer.
//
// The appcast is attached as an asset to every GitHub release. The feed URL
// deliberately points at ".../releases/latest/download/appcast.xml": GitHub
// always redirects that to the newest release, so the URL stays the same
// forever and never has to be maintained.
//
// When WinSparkle is not compiled in (the mingw cross build, or deliberately
// disabled), every call is a no-op and IsSupported() reports false. Callers
// need no special cases for that.
class Updater {
public:
    ~Updater();

    Updater(const Updater&) = delete;
    Updater& operator=(const Updater&) = delete;
    Updater() = default;

    // Was WinSparkle built in?
    static bool IsSupported();

    // Must be called on the message loop thread. `version` is our own version
    // ("0.1.0") and is what the appcast is compared against.
    void Initialize(const std::wstring& version, bool automaticChecks);
    void Shutdown();

    // Manual check with a window -- also reports when everything is current.
    void CheckWithUi();

    void SetAutomaticChecks(bool enabled);

private:
    bool initialized_ = false;
};

}  // namespace wintangle
