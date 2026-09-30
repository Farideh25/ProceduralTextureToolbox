cmake_minimum_required(VERSION 3.16)
project(ProceduralTextureToolbox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# MiniFB: opens a window and displays our pixel buffer (v0.13.0).
set(MINIFB_BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_VERSION_INFO OFF CACHE BOOL "" FORCE)
set(MINIFB_USE_OPENGL_API     OFF CACHE BOOL "" FORCE)  # plain Windows GDI output
FetchContent_Declare(minifb
    GIT_REPOSITORY https://github.com/emoon/minifb.git
    GIT_TAG        a78133e5e5675130e65b9929307ea9e4cab736c0)  # v0.13.0

# MicroUI: tiny immediate-mode UI (v2.02). It has no CMake file, so we compile its source below.
FetchContent_Declare(microui
    GIT_REPOSITORY https://github.com/rxi/microui.git
    GIT_TAG        0850aba860959c3e75fb3e97120ca92957f9d057)  # v2.02

FetchContent_MakeAvailable(minifb microui)

add_executable(ProceduralTextureToolbox
    src/main.cpp
    src/ProceduralTextures.cpp
    src/Ui.cpp
    src/ui_atlas.c
    src/Checks.cpp
    ${microui_SOURCE_DIR}/src/microui.c)

target_include_directories(ProceduralTextureToolbox PRIVATE
    ${microui_SOURCE_DIR}/src    # microui.h
    ${microui_SOURCE_DIR}/demo)  # atlas.inl (MicroUI's built-in font)

target_compile_definitions(ProceduralTextureToolbox PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(ProceduralTextureToolbox PRIVATE minifb)
