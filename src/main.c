#include <stdio.h>
#include "include/6502.h"
#include "include/ppu.h"
#include "include/cartridge.h"

CPU cpu;
PPU ppu;
CPU_Bus cpu_bus;
PPU_Bus ppu_bus;
int cycle_count = 0;
int emulator_running = 0;

int main(int argc, char *argv[]){
	if (argc < 1){
		printf("No file to load from\n");
		return -1;
	}

	init_cpu(&cpu, &cycle_count);
	reset(&cpu, &cpu_bus, &ppu, &ppu_bus);

	Mapper mapper;
	if (load_cartridge(argv[1], &mapper) < 0)
		return -1;
	load_mapper_to_cpu(&cpu, &mapper);

	printf("Starting Emulator...\n");
	while (emulator_running) {
		execute(&cpu);
	}

	exit_cpu(&cpu, argc);
	printf("Exiting Emulator\n %d", cycle_count);
	return 0;
}

