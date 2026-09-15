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
target_compile_definitions(wintangle PRIVATE UNICODE _UNICODE)

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
