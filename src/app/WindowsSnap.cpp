#include "WindowsSnap.h"

#include "platform/Win32.h"

namespace wintangle {
namespace {

constexpr wchar_t kStateKey[] = L"Software\\WinTangle";
// Holds the value the setting had before WinTangle switched it off. Absent
// means WinTangle has not touched it.
constexpr wchar_t kPreviousValue[] = L"WindowArrangingBefore";
constexpr wchar_t kRepairDone[] = L"WindowArrangingRepaired";

// Bumped whenever a released version turned out to leave the setting broken,
// so affected machines get one more attempt. 1 was written by 0.1.0-rc3 and
// rc4, whose repair called the setter the same wrong way that broke it.
constexpr DWORD kRepairVersion = 2;

bool ReadDword(const wchar_t* name, DWORD& out) {
    DWORD size = sizeof(out);
    DWORD type = 0;
    return RegGetValueW(HKEY_CURRENT_USER, kStateKey, name, RRF_RT_REG_DWORD, &type, &out,
                        &size) == ERROR_SUCCESS;
}

bool WriteDword(const wchar_t* name, DWORD value) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kStateKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key,
                        nullptr) != ERROR_SUCCESS) {
        return false;
    }
    const LSTATUS status = RegSetValueExW(key, name, 0, REG_DWORD,
                                          reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}

void DeleteValue(const wchar_t* name) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kStateKey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) {
        return;
    }
    RegDeleteValueW(key, name);
    RegCloseKey(key);
}

// Switches Windows' window arranging on or off, and makes sure it took.
//
// The documentation says the value goes in pvParam and uiParam stays zero.
// That is not what happens on a real machine: earlier versions passed
// pvParam = TRUE with uiParam = 0 and the feature ended up switched OFF, which
// is how users lost Aero Snap and Win+arrow. The value that took effect was
// the one in uiParam.
//
// Rather than bet on either reading, the combinations are tried in turn and
// the result is read back after each. SPIF_UPDATEINIFILE makes the change
// outlast the session -- which matters in both directions.
bool SetWindowArranging(bool enabled) {
    const UINT_PTR value = enabled ? TRUE : FALSE;

    struct Attempt {
        UINT uiParam;
        void* pvParam;
    };
    const Attempt attempts[] = {
        {static_cast<UINT>(value), reinterpret_cast<void*>(value)},  // both, the safe bet
        {static_cast<UINT>(value), nullptr},                         // value in uiParam only
        {0, reinterpret_cast<void*>(value)},                         // as documented
    };

    for (const Attempt& attempt : attempts) {
        SystemParametersInfoW(SPI_SETWINARRANGING, attempt.uiParam, attempt.pvParam,
                              SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        if (IsWindowArrangingEnabled() == enabled) return true;
    }
    return false;
}

}  // namespace

bool IsWindowArrangingEnabled() {
    BOOL enabled = TRUE;
    if (!SystemParametersInfoW(SPI_GETWINARRANGING, 0, &enabled, 0)) return true;
    return enabled != FALSE;
}

void ApplyWindowArrangingPreference(bool disableRequested) {
    DWORD previous = 0;
    const bool weChangedIt = ReadDword(kPreviousValue, previous);

    if (disableRequested) {
        if (weChangedIt) return;  // already off at our request
        WriteDword(kPreviousValue, IsWindowArrangingEnabled() ? 1u : 0u);
        SetWindowArranging(false);
        return;
    }

    if (!weChangedIt) return;  // not ours to touch

    SetWindowArranging(previous != 0);
    DeleteValue(kPreviousValue);
}

bool RepairWindowArrangingIfDamagedByOldVersion(bool disableRequested) {
    // Only for users who do not want it switched off, and only once per repair
    // version.
    if (disableRequested) return true;

    DWORD done = 0;
    if (ReadDword(kRepairDone, done) && done >= kRepairVersion) return true;

    DWORD previous = 0;
    if (ReadDword(kPreviousValue, previous)) return true;  // handled by the normal path

    bool repaired = true;
    if (!IsWindowArrangingEnabled()) repaired = SetWindowArranging(true);

    // Only record the repair when it actually worked. A failed attempt should
    // be retried on the next start rather than written off.
    if (repaired) WriteDword(kRepairDone, kRepairVersion);
    return repaired;
}

}  // namespace wintangle
