#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "include/6502.h"
#include "include/ppu.h"
#include "include/cartridge.h"
#include <sys/time.h>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 480

CPU cpu;
PPU ppu;
CPU_Bus cpu_bus;
PPU_Bus ppu_bus;
int cycle_count = 0;
bool emulator_running = false;
uint64_t current_time;
double FPS;
char FPS_str[12];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;

static int init_emulator(int argc, Mapper *p_mapper, char *argv[]);
static int get_graphics_contexts(void);
static void exit_emulator(int argc);
static void manage_events(SDL_Event *p_event);
static void draw_to_screen(void);
static void update_fps(void);
static uint64_t get_time_us(void);

int main(int argc, char *argv[]){
    int status = get_graphics_contexts();
	if (status < 0) return -1;

	Mapper mapper;
    status = init_emulator(argc, &mapper, argv);
    if (status < 0) return -1;

	printf("Starting Emulator...\n");

	emulator_running = true;
	while (emulator_running) {
        manage_events(&event);
		execute_cpu_ppu(&cpu);
        draw_to_screen();
	}

	printf("Exiting Emulator\n %d", cycle_count);
    exit_emulator(argc);
    
	return 0;
}

static void manage_events(SDL_Event *p_event) {
    SDL_PollEvent(p_event);
    if (event.type == SDL_QUIT) emulator_running = false;
}

static void draw_to_screen(void) {
    if (ppu.frame_complete) {
        SDL_RenderCopy(renderer, (void *)ppu.ppu_draw_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        ppu.frame_complete = false;
        update_fps();
    }
}

static int init_emulator(int argc, Mapper *p_mapper, char *argv[]) {
	if (argc < 1){
		printf("No file to load from\n");
		return -1;
	}

    SDL_Init(SDL_INIT_EVERYTHING);

    reset_cpu(&cpu, &cpu_bus, &ppu);
    reset_ppu(&ppu, &ppu_bus);
    
    int status = load_cartridge(argv[1], p_mapper);
	if (status < 0) return -1;
	load_mapper(&cpu, p_mapper);

	init_cpu(&cpu, &cycle_count);
    status = init_ppu(&ppu, renderer);
    if (status < 0) return -1;

    return 0;
}

static int get_graphics_contexts(void) {
    window = SDL_CreateWindow("NES Emulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT, 0);
    if (window == NULL) return -1;

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) return -1;

    return 0;
}

static void exit_emulator(int argc) {
	exit_cpu(&cpu, argc);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(ppu.ppu_draw_texture);
    SDL_Quit();
}

static void update_fps(void) {
    FPS = 1e6 / (double) (get_time_us() - current_time);
    snprintf(FPS_str, 12, "%.12f", FPS); 
    current_time = get_time_us();
    SDL_SetWindowTitle(window, FPS_str);
}

static uint64_t get_time_us(void) {
	struct timeval current_timeval;
	gettimeofday(&current_timeval, NULL);
	return (uint64_t) current_timeval.tv_sec * (int) 1e6 + current_timeval.tv_usec;
}
