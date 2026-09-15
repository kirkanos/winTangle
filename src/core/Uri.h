#pragma once

#include <string>
#include <string_view>

namespace wintangle {

// Splits "wintangle://execute-action?name=left-half" and returns the action
// name ("left-half"). Empty when the URI does not match. Scheme and host are
// case insensitive and percent encoding is resolved -- the URI arrives from
// elsewhere (Stream Deck, scripts, a browser).
std::string ParseExecuteActionUri(std::string_view uri);

// Percent decoding ("%2D" -> "-", "+" -> " ").
std::string UrlDecode(std::string_view s);

}  // namespace wintangle
