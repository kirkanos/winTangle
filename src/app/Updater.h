#pragma once

#include <string>

namespace wintangle {

// Update-Pruefung ueber WinSparkle -- das Windows-Gegenstueck zu Sparkle, das
// Rectangle benutzt. WinSparkle laedt eine Appcast-XML, vergleicht die
// Version, zeigt die Aenderungen an und startet den Installer.
//
// Die Appcast haengt als Asset an jedem GitHub-Release. Die Feed-Adresse zeigt
// bewusst auf ".../releases/latest/download/appcast.xml": GitHub leitet das
// immer auf das jeweils neueste Release um, die URL bleibt also fuer alle Zeit
// gleich und muss nie mitgepflegt werden.
//
// Ist WinSparkle nicht einkompiliert (mingw-Cross-Build, bewusst abgeschaltet),
// sind alle Aufrufe wirkungslos und IsSupported() meldet false. Der Aufrufer
// braucht dafuer keine Sonderfaelle.
class Updater {
public:
    ~Updater();

    Updater(const Updater&) = delete;
    Updater& operator=(const Updater&) = delete;
    Updater() = default;

    // Wurde WinSparkle mit eingebaut?
    static bool IsSupported();

    // Muss aus dem Thread der Nachrichtenschleife aufgerufen werden.
    // `version` ist die eigene Version ("0.1.0") und wird mit der Appcast
    // verglichen.
    void Initialize(const std::wstring& version, bool automaticChecks);
    void Shutdown();

    // Manuelle Pruefung mit Fenster -- meldet auch, wenn alles aktuell ist.
    void CheckWithUi();

    void SetAutomaticChecks(bool enabled);

private:
    bool initialized_ = false;
};

}  // namespace wintangle
