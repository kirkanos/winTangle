#include "WindowsSnap.h"

#include "platform/Win32.h"

namespace wintangle {
namespace {

constexpr wchar_t kStateKey[] = L"Software\\WinTangle";
// Holds the value the setting had before WinTangle switched it off. Absent
// means WinTangle has not touched it.
constexpr wchar_t kPreviousValue[] = L"WindowArrangingBefore";
constexpr wchar_t kRepairDone[] = L"WindowArrangingRepaired";

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

// SPI_SETWINARRANGING takes the value in pvParam, uiParam stays zero.
// SPIF_UPDATEINIFILE makes the change outlast the session, which matters both
// ways: switching off has to stick, and so does putting it back.
bool SetWindowArranging(bool enabled) {
    const UINT_PTR value = enabled ? TRUE : FALSE;
    SystemParametersInfoW(SPI_SETWINARRANGING, 0, reinterpret_cast<void*>(value),
                          SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    // Read back rather than trust the return value: this is a user visible
    // system setting, and failing silently is how it got broken in the first
    // place.
    return IsWindowArrangingEnabled() == enabled;
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

void RepairWindowArrangingIfDamagedByOldVersion(bool disableRequested) {
    // Only for users who do not want it switched off, and only once.
    if (disableRequested) return;

    DWORD done = 0;
    if (ReadDword(kRepairDone, done) && done != 0) return;

    DWORD previous = 0;
    if (ReadDword(kPreviousValue, previous)) return;  // handled by the normal path

    if (!IsWindowArrangingEnabled()) SetWindowArranging(true);
    WriteDword(kRepairDone, 1u);
}

}  // namespace wintangle
