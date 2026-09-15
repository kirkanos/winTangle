# Die eigentliche Windows-App. Wird nur unter Windows ueberhaupt konfiguriert.

# Versionsressource aus der Vorlage erzeugen. So steht in der Exe genau die
# Version, die in der CMakeLists steht -- der Release-Workflow prueft, dass der
# Git-Tag dazu passt.
set(WINTANGLE_VERSION_COMMA
    "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
configure_file(
    ${CMAKE_SOURCE_DIR}/resources/wintangle.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/wintangle.rc
    @ONLY
)

add_executable(wintangle WIN32
    main.cpp
    Executor.cpp
    ScreenInfo.cpp
    Win32.cpp
    WindowList.cpp
    WindowRef.cpp
    ../app/Autostart.cpp
    ../app/HotkeyManager.cpp
    ../app/Paths.cpp
    ../app/TrayIcon.cpp
    ../app/Updater.cpp
    ../app/UriHandler.cpp
    ../snap/DragTracker.cpp
    ../snap/Footprint.cpp
    ../ui/SettingsWindow.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/wintangle.rc
)

target_link_libraries(wintangle PRIVATE wintangle_core)

target_link_libraries(wintangle PRIVATE
    dwmapi      # DWMWA_EXTENDED_FRAME_BOUNDS, DWMWA_CLOAKED
    shcore      # GetDpiForMonitor
    comctl32    # ListView und Subclassing im Einstellungsdialog
    comdlg32    # Import-/Export-Dateidialog
    shell32     # Tray-Symbol
    user32
    gdi32
    advapi32    # Registry fuer Autostart und URL-Protokoll
    ole32       # SHGetKnownFolderPath
)

# resources/ muss im Include-Pfad des Ressourcen-Compilers liegen, weil die
# erzeugte .rc im Build-Verzeichnis liegt, Icon und Manifest aber nicht.
target_include_directories(wintangle PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/resources
)
target_compile_definitions(wintangle PRIVATE
    UNICODE
    _UNICODE
    # Version aus project() -- dieselbe Quelle wie die Versionsressource, damit
    # Info-Box, Exe-Eigenschaften und Update-Pruefung nie auseinanderlaufen.
    WINTANGLE_VERSION="${PROJECT_VERSION}"
    WINTANGLE_VERSION_W=L"${PROJECT_VERSION}"
)

# ---------------------------------------------------------------------------
# WinSparkle: Update-Pruefung aus dem Programm heraus.
#
# Das offizielle Paket enthaelt nur MSVC-Binaerdateien, deshalb ist die Option
# unter mingw standardmaessig aus. Ohne WinSparkle bleibt das Programm voll
# funktionsfaehig -- Updater.cpp wird dann zu leeren Aufrufen.
# ---------------------------------------------------------------------------
option(WINTANGLE_ENABLE_UPDATES "Update-Pruefung mit WinSparkle einbauen" ${MSVC})

if(WINTANGLE_ENABLE_UPDATES)
    if(NOT MSVC)
        message(FATAL_ERROR
            "WINTANGLE_ENABLE_UPDATES braucht MSVC -- WinSparkle liefert nur "
            "MSVC-Binaerdateien aus. Fuer andere Compiler mit "
            "-DWINTANGLE_ENABLE_UPDATES=OFF bauen.")
    endif()

    include(FetchContent)
    FetchContent_Declare(winsparkle
        URL https://github.com/vslavik/winsparkle/releases/download/v0.9.4/WinSparkle-0.9.4.zip
        URL_HASH SHA256=6037df37fc263bd1650a1c4949681a9d40ffe991d01f35892a406cb5d103c976
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(winsparkle)

    target_include_directories(wintangle PRIVATE ${winsparkle_SOURCE_DIR}/include)
    target_link_libraries(wintangle PRIVATE ${winsparkle_SOURCE_DIR}/x64/Release/WinSparkle.lib)
    target_compile_definitions(wintangle PRIVATE WINTANGLE_WITH_WINSPARKLE)

    # Die DLL muss neben der Exe liegen -- sowohl im Build- als auch im
    # Installationsverzeichnis.
    add_custom_command(TARGET wintangle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${winsparkle_SOURCE_DIR}/x64/Release/WinSparkle.dll
                $<TARGET_FILE_DIR:wintangle>
        COMMENT "WinSparkle.dll neben die Exe kopieren"
    )

    # Fuer das Release-Paket: Pfade nach oben reichen.
    set(WINSPARKLE_DLL ${winsparkle_SOURCE_DIR}/x64/Release/WinSparkle.dll PARENT_SCOPE)
    set(WINSPARKLE_LICENSE ${winsparkle_SOURCE_DIR}/COPYING PARENT_SCOPE)
endif()

if(MINGW)
    # Ohne -municode sucht der Linker main() statt wWinMain(). Der
    # mingw-Build dient nur der Verifikation waehrend der Entwicklung.
    target_compile_options(wintangle PRIVATE -municode -Wall -Wextra)
    target_link_options(wintangle PRIVATE -municode -mwindows)
endif()

if(MSVC)
    target_compile_options(wintangle PRIVATE /W4 /permissive- /utf-8)
    # Einzelne, statisch gelinkte Exe -- kein VC-Redistributable noetig.
    set_property(TARGET wintangle PROPERTY
                 MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
