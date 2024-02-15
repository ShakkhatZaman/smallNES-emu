cmake_minimum_required(VERSION 3.14)
project(emulator)

set(SOURCE_FILES 
	src/main.c 
	src/include/*.c
	src/mappers/*.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
