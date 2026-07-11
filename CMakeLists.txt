cmake_minimum_required(VERSION 3.16)
project(DocTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Raylib configuration
set(RAYLIB_BUILD_EXAMPLES OFF)
set(RAYLIB_BUILD_GAMES OFF)
set(BUILD_SHARED_LIBS OFF)

# Try to find raylib via fetchcontent (recommended for C++)
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

# Also fetch raylib-cpp wrapper
FetchContent_Declare(
    raylib-cpp
    GIT_REPOSITORY https://github.com/raylib-cpp/raylib-cpp.git
    GIT_TAG        5.5.0
)

FetchContent_MakeAvailable(raylib raylib-cpp)

# Find Raylib (installed or fetched)
find_package(raylib CONFIG REQUIRED)
find_package(raylib-cpp CONFIG REQUIRED)

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

target_link_libraries(DocTask PRIVATE
    raylib
    raylib-cpp
)

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