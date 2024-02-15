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

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
