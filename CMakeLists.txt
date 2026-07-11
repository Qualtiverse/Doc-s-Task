cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /WX-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
endif()

# Raylib configuration
set(RAYLIB_BUILD_EXAMPLES OFF)
set(RAYLIB_BUILD_GAMES OFF)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

# Fetch raylib (provides CMake config)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

# Fetch raylib-cpp (header-only, no CMake config)
FetchContent_Declare(
    raylib-cpp
    GIT_REPOSITORY https://github.com/raylib-cpp/raylib-cpp.git
    GIT_TAG        5.5.0
)

FetchContent_MakeAvailable(raylib raylib-cpp)

# Find raylib (from FetchContent or system)
find_package(raylib CONFIG REQUIRED)

# raylib-cpp is header-only - add its include directory directly
target_include_directories(raylib INTERFACE
    ${raylib-cpp_SOURCE_DIR}/include
)

# Executable
add_executable(DocTask
    src/main.cpp
    src/core/Game.cpp
    src/core/TimeRecorder.cpp
    src/core/PhysicsObject.cpp
    src/core/Level.cpp
    src/ui/UIManager.cpp
    src/ui/TimelineUI.cpp
    src/levels/LevelLoader.cpp
    src/levels/Level1.cpp
)

target_include_directories(DocTask PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/levels
)

target_link_libraries(DocTask PRIVATE raylib)

# Copy assets to build directory
add_custom_command(TARGET DocTask POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:DocTask>/assets
    COMMENT "Copying assets to build directory"
)

# Install target (optional)
install(TARGETS DocTask DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/DocTask/assets)