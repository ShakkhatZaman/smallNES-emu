cmake_minimum_required(VERSION 3.14)
project(emulator)

set(SOURCE_FILES 
	src/main.c 
	src/lib/6502.c 
	src/lib/instructions.c
	src/lib/ppu.c
	src/lib/cartridge.c
	src/lib/mapper.c
)

set(HEADER_FILES
	src/lib/6502.h
	src/lib/cartridge.h
	src/lib/instructions.h
	src/lib/mapper.h
	src/lib/ppu.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
