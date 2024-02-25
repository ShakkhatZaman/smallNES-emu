cmake_minimum_required(VERSION 3.14)
project(emulator)

set(SOURCE_FILES 
	src/main.c 
	src/include/6502.c
	src/include/cartridge.c
	src/include/instructions.c
	src/include/mapper.c
	src/include/ppu.c
	src/include/mappers/nrom.c
)

set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

# find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

#include_directories(${SDL2_INCLUDE_DIRS})

#[[ if(TARGET SDL2::SDL2main)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
endif() ]]

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

