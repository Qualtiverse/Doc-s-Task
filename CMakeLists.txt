cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
endif()

# Find raylib - either from vcpkg, FetchContent, or system install
find_package(raylib CONFIG QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(RAYLIB_BUILD_GAMES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )

    FetchContent_MakeAvailable(raylib)

    if(NOT raylib_FOUND)
        find_package(raylib CONFIG REQUIRED)
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

target_link_libraries(DocTask PRIVATE raylib)