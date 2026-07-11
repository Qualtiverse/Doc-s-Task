cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
endif()

# Try finding raylib via CMake config
find_package(raylib CONFIG QUIET)

if(raylib_FOUND)
    message(STATUS "Using raylib from: ${raylib_DIR}")
else()
    message(STATUS "Looking for raylib in CMAKE_PREFIX_PATH: $ENV{CMAKE_PREFIX_PATH}")
    # Manually set up raylib paths - look for it in common locations
    if(DEFINED ENV{RAYLIB_PATH})
        set(RAYLIB_ROOT "$ENV{RAYLIB_PATH}")
    else()
        set(RAYLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/raylib-bin")
    endif()
    
    if(EXISTS "${RAYLIB_ROOT}/include/raylib.h")
        message(STATUS "Found raylib headers at: ${RAYLIB_ROOT}/include")
        add_library(raylib STATIC IMPORTED GLOBAL)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_ROOT}/lib/raylib.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_ROOT}/include"
        )
        set(raylib_FOUND TRUE)
    else()
        message(FATAL_ERROR "raylib not found. Set RAYLIB_PATH or install raylib.")
    endif()
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

target_link_libraries(DocTask PRIVATE raylib winmm)