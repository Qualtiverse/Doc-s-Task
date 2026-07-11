cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try finding raylib via CMake config first
find_package(raylib CONFIG QUIET)

if(NOT raylib_FOUND)
    if(DEFINED ENV{RAYLIB_PATH})
        set(RAYLIB_ROOT "$ENV{RAYLIB_PATH}")
    else()
        set(RAYLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/raylib-bin")
    endif()

    # Find raylib include dir
    find_path(RAYLIB_INCLUDE_DIR raylib.h PATHS "${RAYLIB_ROOT}/include" "${RAYLIB_ROOT}" NO_DEFAULT_PATH)
    if(NOT RAYLIB_INCLUDE_DIR)
        find_path(RAYLIB_INCLUDE_DIR raylib.h)
    endif()

    # Find raylib library (MSVC .lib or MinGW .a)
    find_library(RAYLIB_LIBRARY raylib PATHS "${RAYLIB_ROOT}/lib" "${RAYLIB_ROOT}" NO_DEFAULT_PATH)
    if(NOT RAYLIB_LIBRARY)
        find_library(RAYLIB_LIBRARY raylib)
    endif()

    if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
        message(STATUS "Found raylib: ${RAYLIB_LIBRARY}")
        message(STATUS "  includes: ${RAYLIB_INCLUDE_DIR}")
        add_library(raylib STATIC IMPORTED GLOBAL)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
        )
        set(raylib_FOUND TRUE)
    else()
        message(FATAL_ERROR "raylib not found. Set RAYLIB_PATH or download raylib binaries.")
    endif()
endif()

# Required system libs for raylib on Windows
if(WIN32)
    set(RAYLIB_SYSTEM_LIBS winmm gdi32 opengl32)
else()
    set(RAYLIB_SYSTEM_LIBS)
endif()

add_executable(DocTask
    src/main.cpp
    src/core/CoreTypes.cpp
    src/core/GameManager.cpp
    src/core/Level.cpp
    src/ui/UIManager.cpp
    src/levels/Levels.cpp
)

target_include_directories(DocTask PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/levels
)

target_link_libraries(DocTask PRIVATE raylib ${RAYLIB_SYSTEM_LIBS})