#pragma once

#include <string>
#include <string_view>

namespace wintangle {

// Zerlegt "wintangle://execute-action?name=left-half" und gibt den
// Aktionsnamen zurueck ("left-half"). Leer, wenn die URI nicht passt.
// Gross-/Kleinschreibung von Schema und Host ist egal, Prozent-Kodierung wird
// aufgeloest -- die URI kommt aus fremder Hand (Streamdeck, Skripte, Browser).
std::string ParseExecuteActionUri(std::string_view uri);

// Prozent-Dekodierung ("%2D" -> "-", "+" -> " ").
std::string UrlDecode(std::string_view s);

}  // namespace wintangle
