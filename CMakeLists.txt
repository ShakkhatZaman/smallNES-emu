cmake_minimum_required(VERSION 3.14)
project(emulator)

set(SOURCE_FILES 
	main.c 
	lib/6502.c 
	lib/ins_functions.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
