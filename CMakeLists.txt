cmake_minimum_required(VERSION 3.10)
project(quoridor_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add all source files
add_executable(quoridor_server
    src/main.cpp
    src/player.cpp
    src/quoridor_game.cpp
    src/quoridor_server.cpp
)

# Link against pthread for thread support
target_link_libraries(quoridor_server pthread) 