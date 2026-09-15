cmake_minimum_required(VERSION 3.20)
project(WinTangle VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# wintangle_core: die komplette Geometrie, plattformfrei.
# Baut und testet auch auf macOS/Linux -- genau dafuer ist der Kern getrennt.
# ---------------------------------------------------------------------------
add_library(wintangle_core STATIC
    src/core/Action.cpp
    src/core/Calculation.cpp
    src/core/History.cpp
    src/core/Json.cpp
    src/core/Layout.cpp
    src/core/Shortcut.cpp
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
    tests/test_uri.cpp
)
target_link_libraries(wintangle_tests PRIVATE wintangle_core)
target_include_directories(wintangle_tests PRIVATE tests)
add_test(NAME core COMMAND wintangle_tests)

# ---------------------------------------------------------------------------
# Die eigentliche App -- nur unter Windows.
# ---------------------------------------------------------------------------
if(WIN32)
    add_subdirectory(src/platform)
endif()
