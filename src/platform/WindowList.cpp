#include "WindowList.h"

namespace wintangle {
namespace {

struct EnumContext {
    const WindowFilter* filter;
    std::vector<WindowRef>* out;
};

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM param) {
    auto* ctx = reinterpret_cast<EnumContext*>(param);
    if (ctx->out->size() >= ctx->filter->maxCount) return FALSE;  // genug gesammelt

    WindowRef w(hwnd);
    if (w.IsMinimized() && !ctx->filter->includeMinimized) return TRUE;
    if (!w.IsManageable()) return TRUE;

    if (ctx->filter->monitor &&
        MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) != ctx->filter->monitor) {
        return TRUE;
    }
    if (ctx->filter->processId && ProcessIdOf(hwnd) != ctx->filter->processId) return TRUE;

    ctx->out->push_back(w);
    return TRUE;
}

}  // namespace

DWORD ProcessIdOf(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

std::vector<WindowRef> ListWindows(const WindowFilter& filter) {
    std::vector<WindowRef> out;
    // EnumWindows liefert bereits Z-Order von vorne nach hinten.
    EnumContext ctx{&filter, &out};
    EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&ctx));
    return out;
}

}  // namespace wintangle
