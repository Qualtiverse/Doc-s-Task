cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
endif()

# Try to find pre-built raylib first, else build from source
find_package(raylib CONFIG QUIET)

if(raylib_FOUND)
    message(STATUS "Found pre-built raylib")
else()
    message(STATUS "Building raylib from source")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(raylib-src raylib-build)
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