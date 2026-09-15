#pragma once

#include <string>

#include "Win32.h"

namespace wintangle {

// Duenne Huelle um ein HWND. Kuemmert sich um die beiden Dinge, an denen unter
// Windows jedes Fenster-Tool zuerst scheitert:
//
//  * Der unsichtbare Schattenrand. GetWindowRect liefert ihn mit, sichtbar ist
//    aber DWMWA_EXTENDED_FRAME_BOUNDS. Ohne Korrektur sitzt jedes Fenster um
//    ein paar Pixel daneben und zwei "Haelften" ueberlappen sich.
//  * Maximierte Fenster. SetWindowPos auf ein maximiertes Fenster ergibt einen
//    inkonsistenten Zustand; es muss vorher wiederhergestellt werden.
class WindowRef {
public:
    WindowRef() = default;
    explicit WindowRef(HWND hwnd) : hwnd_(hwnd) {}

    HWND Handle() const { return hwnd_; }
    explicit operator bool() const { return hwnd_ != nullptr && IsWindow(hwnd_); }
    std::uint64_t Id() const { return reinterpret_cast<std::uint64_t>(hwnd_); }

    // Sichtbarer Rahmen (schattenkorrigiert). Das ist die Groesse, die der
    // Benutzer sieht und mit der der Berechnungskern arbeitet.
    Rect Frame() const;

    // Setzt den sichtbaren Rahmen. Gibt false zurueck, wenn Windows das
    // ablehnt -- typischerweise bei Fenstern erhoehter Prozesse (UIPI).
    bool SetFrame(const Rect& frame, DWORD* errorOut = nullptr);

    // Rechnet einen gewuenschten sichtbaren Rahmen in die Koordinaten um, die
    // SetWindowPos/DeferWindowPos erwarten. Wird vom Mehrfenster-Pfad
    // gebraucht, der alle Fenster in einem Rutsch setzt.
    RECT ToWindowRect(const Rect& frame) const;

    bool IsMaximized() const { return IsZoomed(hwnd_) != FALSE; }
    bool IsMinimized() const { return IsIconic(hwnd_) != FALSE; }
    bool IsCloaked() const;

    // Kann und soll WinTangle dieses Fenster anfassen? Filtert Desktop, Shell,
    // Werkzeugfenster, unsichtbare und von der Shell "gecloakte" Fenster.
    bool IsManageable() const;

    // Dateiname der zugehoerigen Exe in Kleinschreibung ("notepad.exe").
    std::string ExeName() const;
    std::wstring Title() const;

    // Holt das Fenster nach vorn, ohne den Fokus zu stehlen, wenn es das schon ist.
    void EnsureRestored();

private:
    // Differenz zwischen Fensterrechteck und sichtbarem Rahmen, pro Kante.
    RECT ShadowInsets() const;

    HWND hwnd_ = nullptr;
};

}  // namespace wintangle
