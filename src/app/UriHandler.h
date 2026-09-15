#pragma once

#include <functional>
#include <string>
#include <vector>

#include "platform/Win32.h"

namespace wintangle {

// wintangle://execute-action?name=left-half
//
// Gegenstueck zu Rectangles rectangle://-Schema. Zwei Aufgaben:
//   * Das Protokoll in der Registry anmelden (HKCU, keine Adminrechte).
//   * Aufrufe an die bereits laufende Instanz weiterreichen -- Windows
//     startet fuer jeden Protokoll-Aufruf einen neuen Prozess, der seine
//     Kommandozeile per Named Pipe an die erste Instanz schickt und sich
//     danach sofort beendet.
class UriServer {
public:
    // Wird im Thread der Nachrichtenschleife aufgerufen, nicht im Pipe-Thread.
    using Handler = std::function<void(const std::string& uri)>;

    ~UriServer();

    // Startet den Lauscher. `window` bekommt kMsgUriCommand gepostet, sobald
    // ein Aufruf hereinkommt.
    bool Start(HWND window);
    void Stop();

    // Holt die naechste eingegangene URI ab (aus dem Nachrichten-Thread).
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

// Schickt eine URI an eine laufende Instanz. Gibt false zurueck, wenn keine
// laeuft.
bool SendUriToRunningInstance(const std::string& uri);

// Registriert bzw. entfernt das Protokoll fuer den aktuellen Benutzer.
bool RegisterUriScheme();
bool UnregisterUriScheme();

}  // namespace wintangle
