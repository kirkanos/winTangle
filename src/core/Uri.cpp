#include "Uri.h"

#include <algorithm>
#include <cctype>

namespace wintangle {
namespace {

bool StartsWithIgnoreCase(std::string_view text, std::string_view prefix) {
    if (text.size() < prefix.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), text.begin(), [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) ==
               std::tolower(static_cast<unsigned char>(b));
    });
}

int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

}  // namespace

std::string UrlDecode(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            const int hi = HexValue(s[i + 1]);
            const int lo = HexValue(s[i + 2]);
            if (hi < 0 || lo < 0) {
                out += s[i];
            } else {
                out += static_cast<char>(hi * 16 + lo);
                i += 2;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string ParseExecuteActionUri(std::string_view uri) {
    constexpr std::string_view kScheme = "wintangle://";
    if (!StartsWithIgnoreCase(uri, kScheme)) return {};
    uri.remove_prefix(kScheme.size());

    // Windows haengt an Protokoll-Aufrufe gerne einen Schraegstrich an.
    while (!uri.empty() && uri.back() == '/') uri.remove_suffix(1);

    const size_t query = uri.find('?');
    const std::string_view host = uri.substr(0, query);
    if (!StartsWithIgnoreCase(host, "execute-action") || host.size() != 14) return {};
    if (query == std::string_view::npos) return {};

    std::string_view params = uri.substr(query + 1);
    while (!params.empty()) {
        const size_t amp = params.find('&');
        const std::string_view pair = params.substr(0, amp);
        params = amp == std::string_view::npos ? std::string_view{} : params.substr(amp + 1);

        const size_t eq = pair.find('=');
        if (eq == std::string_view::npos) continue;
        if (!StartsWithIgnoreCase(pair.substr(0, eq), "name") || eq != 4) continue;

        std::string value = UrlDecode(pair.substr(eq + 1));
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
    return {};
}

}  // namespace wintangle
