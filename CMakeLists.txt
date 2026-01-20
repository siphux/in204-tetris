cmake_minimum_required(VERSION 3.11)
project(IN204-TETRIS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

set(SFML_BUILD_AUDIO ON CACHE BOOL "Build SFML audio module" FORCE)
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "Build SFML graphics module" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "Build SFML window module" FORCE)
set(SFML_BUILD_SYSTEM ON CACHE BOOL "Build SFML system module" FORCE)
set(SFML_BUILD_NETWORK ON CACHE BOOL "Build SFML network module" FORCE)


FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
)


FetchContent_MakeAvailable(SFML)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-network
    sfml-audio
)