cmake_minimum_required(VERSION 3.20)
project(WinTangle VERSION 0.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# A single statically linked executable -- no VC redistributable to install.
# This has to be set for every target, not just the app: mixing a static and a
# dynamic CRT within one binary is a link error (LNK2038), and the core library
# and the tests are linked into it.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ---------------------------------------------------------------------------
# wintangle_core: the entire geometry, platform free.
# Builds and tests on macOS and Linux too -- which is exactly why it is split
# off from the Win32 layer.
# ---------------------------------------------------------------------------
add_library(wintangle_core STATIC
    src/core/Action.cpp
    src/core/ActionGlyph.cpp
    src/core/Calculation.cpp
    src/core/History.cpp
    src/core/Json.cpp
    src/core/Layout.cpp
    src/core/Shortcut.cpp
    src/core/Strings.cpp
    src/core/SnapZones.cpp
    src/core/Uri.cpp
    src/config/Config.cpp
)
target_include_directories(wintangle_core PUBLIC src)

if(MSVC)
    target_compile_options(wintangle_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(wintangle_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_executable(wintangle_tests
    tests/main.cpp
    tests/test_core.cpp
    tests/test_config.cpp
    tests/test_glyph.cpp
    tests/test_layout.cpp
    tests/test_strings.cpp
    tests/test_uri.cpp
)
target_link_libraries(wintangle_tests PRIVATE wintangle_core)
target_include_directories(wintangle_tests PRIVATE tests)
add_test(NAME core COMMAND wintangle_tests)

# ---------------------------------------------------------------------------
# Developer tool: dumps the menu glyph data so tools/preview_glyphs.py can
# rasterise it. Not part of the app, and built on any platform.
# ---------------------------------------------------------------------------
add_executable(dump_glyphs EXCLUDE_FROM_ALL tools/dump_glyphs.cpp)
target_link_libraries(dump_glyphs PRIVATE wintangle_core)

# ---------------------------------------------------------------------------
# The actual app -- Windows only.
# ---------------------------------------------------------------------------
if(WIN32)
    add_subdirectory(src/platform)
endif()
