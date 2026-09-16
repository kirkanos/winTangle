#include "UriHandler.h"

#include "Commands.h"
#include "Paths.h"

namespace wintangle {
namespace {

// One pipe name per session is enough: WinTangle runs per user.
constexpr wchar_t kPipeName[] = L"\\\\.\\pipe\\WinTangle.v1";
constexpr DWORD kMaxMessage = 2048;

constexpr wchar_t kSchemeKey[] = L"Software\\Classes\\wintangle";
constexpr wchar_t kSchemeCommandKey[] = L"Software\\Classes\\wintangle\\shell\\open\\command";

bool WriteRegString(HKEY root, const wchar_t* subkey, const wchar_t* name,
                    const std::wstring& value) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(root, subkey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) !=
        ERROR_SUCCESS) {
        return false;
    }
    const LSTATUS status =
        RegSetValueExW(key, name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()),
                       static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}

}  // namespace

UriServer::~UriServer() { Stop(); }

bool UriServer::Start(HWND window) {
    window_ = window;
    InitializeCriticalSection(&lock_);
    lockReady_ = true;

    stopEvent_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!stopEvent_) return false;

    thread_ = CreateThread(
        nullptr, 0,
        [](LPVOID param) -> DWORD {
            static_cast<UriServer*>(param)->ThreadMain();
            return 0;
        },
        this, 0, nullptr);
    return thread_ != nullptr;
}

void UriServer::Stop() {
    if (stopEvent_) SetEvent(stopEvent_);
    if (thread_) {
        // The pipe blocks in ConnectNamedPipe; a dummy connect releases it.
        HANDLE dummy = CreateFileW(kPipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (dummy != INVALID_HANDLE_VALUE) CloseHandle(dummy);
        WaitForSingleObject(thread_, 2000);
        CloseHandle(thread_);
        thread_ = nullptr;
    }
    if (stopEvent_) {
        CloseHandle(stopEvent_);
        stopEvent_ = nullptr;
    }
    if (lockReady_) {
        DeleteCriticalSection(&lock_);
        lockReady_ = false;
    }
}

void UriServer::ThreadMain() {
    while (WaitForSingleObject(stopEvent_, 0) != WAIT_OBJECT_0) {
        HANDLE pipe = CreateNamedPipeW(kPipeName, PIPE_ACCESS_INBOUND,
                                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
                                       0, kMaxMessage, 0, nullptr);
        if (pipe == INVALID_HANDLE_VALUE) {
            Sleep(250);
            continue;
        }

        const BOOL connected =
            ConnectNamedPipe(pipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (connected && WaitForSingleObject(stopEvent_, 0) != WAIT_OBJECT_0) {
            char buffer[kMaxMessage]{};
            DWORD read = 0;
            if (ReadFile(pipe, buffer, sizeof(buffer) - 1, &read, nullptr) && read > 0) {
                EnterCriticalSection(&lock_);
                pending_.emplace_back(buffer, read);
                LeaveCriticalSection(&lock_);
                // Handling belongs on the UI thread; just wake it up here.
                PostMessageW(window_, kMsgUriCommand, 0, 0);
            }
        }
        DisconnectNamedPipe(pipe);
        CloseHandle(pipe);
    }
}

bool UriServer::PopPending(std::string& uri) {
    if (!lockReady_) return false;
    EnterCriticalSection(&lock_);
    const bool have = !pending_.empty();
    if (have) {
        uri = pending_.front();
        pending_.erase(pending_.begin());
    }
    LeaveCriticalSection(&lock_);
    return have;
}

bool SendUriToRunningInstance(const std::string& uri) {
    // Wait briefly in case the first instance is only just coming up.
    if (!WaitNamedPipeW(kPipeName, 1000)) return false;

    HANDLE pipe = CreateFileW(kPipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    const BOOL ok = WriteFile(pipe, uri.data(), static_cast<DWORD>(uri.size()), &written, nullptr);
    CloseHandle(pipe);
    return ok && written == uri.size();
}

// Reads a string value; empty when it is missing.
std::wstring ReadRegString(HKEY root, const wchar_t* subkey, const wchar_t* name) {
    wchar_t buffer[1024]{};
    DWORD size = sizeof(buffer);
    DWORD type = 0;
    if (RegGetValueW(root, subkey, name, RRF_RT_REG_SZ, &type, buffer, &size) != ERROR_SUCCESS) {
        return {};
    }
    return buffer;
}

bool RegisterUriScheme() {
    const std::wstring command = L"\"" + ExecutablePath() + L"\" \"%1\"";

    // Already registered and pointing at this executable: leave the registry
    // alone. Rewriting it on every start is needless, and a program that keeps
    // rewriting registry keys looks worse to a scanner than it deserves.
    if (ReadRegString(HKEY_CURRENT_USER, kSchemeCommandKey, nullptr) == command) return true;
    bool ok = WriteRegString(HKEY_CURRENT_USER, kSchemeKey, nullptr, L"URL:WinTangle Protocol");
    ok = WriteRegString(HKEY_CURRENT_USER, kSchemeKey, L"URL Protocol", L"") && ok;
    ok = WriteRegString(HKEY_CURRENT_USER, kSchemeCommandKey, nullptr, command) && ok;
    return ok;
}

bool UnregisterUriScheme() {
    return RegDeleteTreeW(HKEY_CURRENT_USER, kSchemeKey) == ERROR_SUCCESS;
}

}  // namespace wintangle
