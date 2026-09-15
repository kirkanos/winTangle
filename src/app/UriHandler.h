#pragma once

#include <functional>
#include <string>
#include <vector>

#include "platform/Win32.h"

namespace wintangle {

// wintangle://execute-action?name=left-half
//
// The counterpart to Rectangle's rectangle:// scheme. Two jobs:
//   * Register the protocol in the registry (HKCU, no admin rights).
//   * Hand calls over to the instance that is already running -- Windows
//     starts a fresh process for every protocol invocation, which forwards
//     its command line through a named pipe and then exits immediately.
class UriServer {
public:
    // Called on the message loop thread, not on the pipe thread.
    using Handler = std::function<void(const std::string& uri)>;

    ~UriServer();

    // Starts the listener. `window` gets kMsgUriCommand posted as soon as a
    // call comes in.
    bool Start(HWND window);
    void Stop();

    // Fetches the next URI that came in (from the message thread).
    bool PopPending(std::string& uri);

private:
    void ThreadMain();

    HWND window_ = nullptr;
    HANDLE thread_ = nullptr;
    HANDLE stopEvent_ = nullptr;
    CRITICAL_SECTION lock_{};
    bool lockReady_ = false;
    std::vector<std::string> pending_;
};

// Sends a URI to a running instance. Returns false when none is running.
bool SendUriToRunningInstance(const std::string& uri);

// Registers or removes the protocol for the current user.
bool RegisterUriScheme();
bool UnregisterUriScheme();

}  // namespace wintangle
