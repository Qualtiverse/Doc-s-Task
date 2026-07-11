cmake_minimum_required(VERSION 3.16)
project(LevelEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib CONFIG QUIET)

if(NOT raylib_FOUND)
    if(DEFINED ENV{RAYLIB_PATH})
        set(RAYLIB_ROOT "$ENV{RAYLIB_PATH}")
    else()
        set(RAYLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../raylib-bin")
    endif()

    find_path(RAYLIB_INCLUDE_DIR raylib.h PATHS "${RAYLIB_ROOT}/include" "${RAYLIB_ROOT}" NO_DEFAULT_PATH)
    if(NOT RAYLIB_INCLUDE_DIR)
        find_path(RAYLIB_INCLUDE_DIR raylib.h)
    endif()

    find_library(RAYLIB_LIBRARY raylib PATHS "${RAYLIB_ROOT}/lib" "${RAYLIB_ROOT}" NO_DEFAULT_PATH)
    if(NOT RAYLIB_LIBRARY)
        find_library(RAYLIB_LIBRARY raylib)
    endif()

    if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
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

add_executable(LevelEditor LevelEditor.cpp)

if(WIN32)
    target_link_libraries(LevelEditor PRIVATE raylib winmm gdi32 opengl32)
else()
    target_link_libraries(LevelEditor PRIVATE raylib)
endif()
