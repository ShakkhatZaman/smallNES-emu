#include <stdbool.h>
#include <sys/time.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "utils.h"
#include "emulator/6502/6502.h"
#include "emulator/ppu/ppu.h"
#include "emulator/cartridge/mapper.h"
#include "emulator/cartridge/cartridge.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 480

typedef struct timer {
    uint64_t start_time;
    uint64_t duration;
} timer;

CPU cpu = {.p_Bus = NULL};
PPU ppu = {.p_Bus = NULL};
CPU_Bus cpu_bus;
PPU_Bus ppu_bus;
int64_t cycle_count = 0;
bool emulator_running = false;
uint64_t current_time;
double FPS;
char FPS_str[12];
timer fps_timer = {
    .duration = 200000,
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;

static int init_emulator(int argc, Mapper *p_mapper, char *argv[]);
static int get_graphics_contexts(void);
static void exit_emulator(void);
static void manage_events(SDL_Event *p_event);
static void draw_to_screen(void);
static void update_fps(void);
static uint64_t get_time_us(void);

int main(int argc, char *argv[]){
    Mapper mapper = (Mapper) {
        .PRG_ROM_banks = 0, .CHR_ROM_banks = 0,
        .PRG_ROM_p = NULL, .CHR_ROM_p = NULL,
    };

    int status = init_emulator(argc, &mapper, argv);
    if (status != 0) {
        exit_emulator();
        if (status < 0) ERROR_EXIT("Unable to initialize emulator, Exited program with status: %d", status);
        return status;
    };

    status = get_graphics_contexts();
    if (status < 0) {
        exit_emulator();
        ERROR_EXIT("Unable to get graphics contexts, Exited program with status: %d", status);
    }

    emulator_running = true;
    fps_timer.start_time = get_time_us();
    while (emulator_running) {
        manage_events(&event);
        execute_cpu_ppu(&cpu);
        draw_to_screen();
    }

    exit_emulator();
    return status;
}

static void manage_events(SDL_Event *p_event) {
    SDL_PollEvent(p_event);
    if (event.type == SDL_QUIT) emulator_running = false;
}

static void draw_to_screen(void) {
    if (ppu.frame_complete) {
        if(SDL_RenderCopy(renderer, (void *)ppu.ppu_draw_texture, NULL, NULL) < 0) {
            ERROR("Unable to draw to screen\n    SDL error: %s", SDL_GetError());
            emulator_running = false;
        }
        SDL_RenderPresent(renderer);
        ppu.frame_complete = false;
        update_fps();
    }
}

static int init_emulator(int argc, Mapper *p_mapper, char *argv[]) {
    printf("Starting Emulator...\n");

    if (argc < 2){
        printf("No file to load from\n");
        return 1;
    }

    int status = SDL_Init(SDL_INIT_EVERYTHING);
    if (status < 0)
        ERROR_RETURN("Unable to initialize SDL (flags: %d)\n    SDL error: %s", SDL_INIT_EVERYTHING, SDL_GetError());

    reset_cpu(&cpu, &cpu_bus, &ppu);
    reset_ppu(&ppu, &ppu_bus);
    load_mapper(&cpu, p_mapper);

    status = load_cartridge(argv[1], p_mapper);
    if (status < 0)
        ERROR_RETURN("Unable to load NES cartridge %s", argv[1]);

    init_cpu(&cpu, &cycle_count);

    return 0;
}

static int get_graphics_contexts(void) {
    window = SDL_CreateWindow("NES Emulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT, 0);
    if (window == NULL)
        ERROR_RETURN("Unable to create Window (width: %d, height: %d, flags: %d)", WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
        ERROR_RETURN("Unable to create Renderer (index: %d, flags: %d)", -1, 0);
    
    int status = init_ppu(&ppu, renderer);
    if (status < 0)
        ERROR_RETURN("Unable to initialize PPU\n    SDL error: %s", SDL_GetError());

    return 0;
}

static void exit_emulator(void) {
    printf("Exiting Emulator\nCycle count: %d\n", cycle_count);
    exit_cpu(&cpu);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(ppu.ppu_draw_texture);
    SDL_Quit();
}

static void update_fps(void) {
    if (get_time_us() >= (fps_timer.start_time + fps_timer.duration)) {
        FPS = 1e6 / (double) (get_time_us() - current_time);
        snprintf(FPS_str, 11, "%.12f", FPS); 
        SDL_SetWindowTitle(window, FPS_str);
        fps_timer.start_time = get_time_us();
    }
    current_time = get_time_us();
}

static uint64_t get_time_us(void) {
    struct timeval current_timeval;
    gettimeofday(&current_timeval, NULL);
    return (uint64_t) current_timeval.tv_sec * (int) 1e6 + current_timeval.tv_usec;
}
