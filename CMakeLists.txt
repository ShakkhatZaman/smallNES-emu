cmake_minimum_required(VERSION 3.14)
project(emulator)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES 
    "src/main.c"
    "src/emulator/6502/6502.c"
    "src/emulator/6502/instructions.c"
    "src/emulator/ppu/ppu.c"
    "src/emulator/cartridge/mapper.c"
    "src/emulator/cartridge/cartridge.c"
    "src/emulator/cartridge/mappers/nrom.c"
)

set(LINKING_LIBRARIES
    "SDL2"
    "SDL2main"
    "SDL2_test"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE "./src/include/")
target_link_directories(${PROJECT_NAME} PRIVATE "./src/lib/")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINKING_LIBRARIES})

