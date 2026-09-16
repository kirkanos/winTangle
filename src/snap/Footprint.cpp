#include "Footprint.h"

namespace wintangle {
namespace {

constexpr wchar_t kClassName[] = L"WinTangleFootprint";
constexpr BYTE kAlpha = 140;
constexpr COLORREF kFill = RGB(0x3A, 0x7B, 0xD5);
constexpr COLORREF kBorder = RGB(0xFF, 0xFF, 0xFF);

}  // namespace

Footprint::~Footprint() {
    if (hwnd_) DestroyWindow(hwnd_);
}

LRESULT CALLBACK Footprint::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* self = reinterpret_cast<Footprint*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                              reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(hwnd, &ps);
            RECT client{};
            GetClientRect(hwnd, &client);
            if (self) self->Paint(dc, client);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;  // flicker free: everything happens in WM_PAINT
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        default:
            return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

void Footprint::Paint(HDC dc, const RECT& client) const {
    HBRUSH fill = CreateSolidBrush(kFill);
    FillRect(dc, &client, fill);
    DeleteObject(fill);

    // A clearly visible border, so the preview reads on light and dark
    // backgrounds alike.
    HBRUSH border = CreateSolidBrush(kBorder);
    RECT frame = client;
    FrameRect(dc, &frame, border);
    InflateRect(&frame, -1, -1);
    FrameRect(dc, &frame, border);
    DeleteObject(border);
}

bool Footprint::Create(HINSTANCE instance) {
    if (hwnd_) return true;  // switching snap areas off and on again must not leak a window

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);  // registering twice is harmless

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        kClassName, L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, this);
    if (!hwnd_) return false;

    SetLayeredWindowAttributes(hwnd_, 0, kAlpha, LWA_ALPHA);
    return true;
}

void Footprint::ShowAt(const Rect& frame) {
    if (!hwnd_ || frame.IsEmpty()) return;
    if (visible_ && frame == current_) return;  // avoid needless repainting

    current_ = frame;
    SetWindowPos(hwnd_, HWND_TOPMOST, frame.left, frame.top, frame.Width(), frame.Height(),
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(hwnd_, nullptr, FALSE);
    visible_ = true;
}

void Footprint::Hide() {
    if (!hwnd_ || !visible_) return;
    ShowWindow(hwnd_, SW_HIDE);
    visible_ = false;
}

}  // namespace wintangle
