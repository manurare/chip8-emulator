cmake_minimum_required(VERSION 3.10)

# set the project name
project(chip8)

find_package (sdl2 PATHS /home/manuel/libraries/SDL/lib/cmake/SDL2)

# add the executable
add_executable(chip8 main.cpp src/Chip8.cpp src/Chip8.h)
target_include_directories(chip8 PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8 ${SDL2_LIBRARIES})
