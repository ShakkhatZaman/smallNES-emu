#include <stdio.h>
#include "lib/6502.h"
#include "lib/ppu.h"
#include "lib/cartridge.h"

int main(int argc, char *argv[]){
	int cycle_count;
	printf("Enter number of clock cycles: \n");
	scanf("%d", &cycle_count);

	printf("Initializing CPU...\n");
	struct CPU cpu;

	printf("Initializing Memory...\n");
	CPU_Bus cpu_bus;
	PPU ppu;
	PPU_Bus ppu_bus;

	printf("Reseting CPU...\n");
	reset(&cpu, &cpu_bus, &ppu, &ppu_bus);
	
	if (argc > 1){
		Mapper mapper;
		load_cartridge(argv[1], &mapper);
		load_mapper_to_cpu(&cpu, &mapper);
	}

	printf("Starting Execution...\n");
	execute(&cpu, cycle_count);
	
	exit_cpu(&cpu, argc);
	printf("Exiting Emulator\n");
	return 0;
}

