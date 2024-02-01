#include <stdio.h>
#include "lib/6502.h"

int main(int argc, char *argv[]){
	int cycle_count;
	printf("Enter number of clock cycles: \n");
	scanf("%d", &cycle_count);

	printf("Initializing CPU...\n");
	struct CPU cpu;

	printf("Initializing Memory...\n");
	struct Mem mem;

	printf("Reseting CPU...\n");
	reset(&cpu, &mem);
	
	printf("Starting Execution...\n");
	execute(&cpu, cycle_count);
	
	printf("%d %d %d\n", cpu.A, mem.Data[0x01ff], mem.Data[0x01fe]);

	printf("Exiting Emulator\n");
	return 0;
}

