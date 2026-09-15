# The actual Windows app. Only configured on Windows in the first place.

# Generate the version resource from the template, so the executable carries
# exactly the version declared in this CMakeLists -- the release workflow
# checks that the git tag agrees.
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
    ../app/Cleanup.cpp
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
    comctl32    # list view and subclassing in the settings window
    comdlg32    # import/export file dialog
    shell32     # tray icon
    user32
    gdi32
    advapi32    # registry for autostart and the URL protocol
    ole32       # SHGetKnownFolderPath
)

# resources/ has to be on the resource compiler's include path, because the
# generated .rc lives in the build directory while icon and manifest do not.
target_include_directories(wintangle PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/resources
)
target_compile_definitions(wintangle PRIVATE
    UNICODE
    _UNICODE
    # Version from project() -- the same source as the version resource, so the
    # about box, the file properties and the update check never diverge.
    WINTANGLE_VERSION="${PROJECT_VERSION}"
    WINTANGLE_VERSION_W=L"${PROJECT_VERSION}"
)

# ---------------------------------------------------------------------------
# WinSparkle: in-app update checking.
#
# The official package ships MSVC binaries only, which is why the option is off
# by default under mingw. Without WinSparkle the program remains fully
# functional -- Updater.cpp then compiles down to empty calls.
# ---------------------------------------------------------------------------
option(WINTANGLE_ENABLE_UPDATES "Build with WinSparkle update checking" ${MSVC})

if(WINTANGLE_ENABLE_UPDATES)
    if(NOT MSVC)
        message(FATAL_ERROR
            "WINTANGLE_ENABLE_UPDATES requires MSVC -- WinSparkle ships MSVC "
            "binaries only. Build other compilers with "
            "-DWINTANGLE_ENABLE_UPDATES=OFF.")
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

    # The DLL has to sit next to the executable, in the build tree as well as
    # in the installation directory.
    add_custom_command(TARGET wintangle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${winsparkle_SOURCE_DIR}/x64/Release/WinSparkle.dll
                $<TARGET_FILE_DIR:wintangle>
        COMMENT "Copying WinSparkle.dll next to the executable"
    )

    # Hand the paths up for the release package.
    set(WINSPARKLE_DLL ${winsparkle_SOURCE_DIR}/x64/Release/WinSparkle.dll PARENT_SCOPE)
    set(WINSPARKLE_LICENSE ${winsparkle_SOURCE_DIR}/COPYING PARENT_SCOPE)
endif()

if(MINGW)
    # Without -municode the linker looks for main() instead of wWinMain(). The
    # mingw build exists purely for verification during development.
    target_compile_options(wintangle PRIVATE -municode -Wall -Wextra)
    target_link_options(wintangle PRIVATE -municode -mwindows)
endif()

if(MSVC)
    target_compile_options(wintangle PRIVATE /W4 /permissive- /utf-8)
    # A single statically linked executable -- no VC redistributable needed.
    set_property(TARGET wintangle PROPERTY
                 MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
