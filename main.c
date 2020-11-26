#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#if OSX
#  include <SDL2/SDL.h>
#else
#  include <SDL.h>
#endif

#include "main.h"
#include "tetris.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

enum {
    DEFAULT_SCREEN_WIDTH =  640,
    DEFAULT_SCREEN_HEIGHT = 480,
};

static SDL_Window * win;
static SDL_Renderer * ren;

int done = 0;
int win_w = 0;
int win_h = 0;
int sq = 0;

int use_seed = 1;
int seed = 0;

static void
set_w_h()
{
    SDL_DisplayMode mode;
    if (SDL_GetWindowDisplayMode(win, &mode)) {
        fprintf(stderr, "SDL_GetWindowDisplayMode(): %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    win_w = mode.w;
    win_h = mode.h;
    sq = win_h / 22;
}

void
toggle_fullscreen()
{
    SDL_SetWindowFullscreen(win, SDL_GetWindowFlags(win)^SDL_WINDOW_FULLSCREEN_DESKTOP);
    set_w_h();
}

void
init_program()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    uint32_t flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    win = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, flags);

    if (win == NULL) {
        fprintf(stderr, "SDL_CreateWindow(): %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    set_w_h();

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (ren == NULL) {
        fprintf(stderr, "SDL_CreateRenderer(): %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

struct timespec
gettime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime()");
        exit(EXIT_FAILURE);
    }
    return ts;
}

long long int
diff_ns(struct timespec end, struct timespec begin)
{
    long long int sec = end.tv_sec - begin.tv_sec;
    long long int nsec = end.tv_nsec - begin.tv_nsec;
    return sec*1000000000 + nsec;
}

void 
loop()
{
    uint32_t ticks_start, ticks_end;
    uint32_t frame_num = 0, num_dropped = 0;
    while (!done) {
        const uint32_t ms_per_frame = (int) (1000.0/FPS);
        uint32_t sleep_dur, used_time;
        int dropped;
        UNUSED(sleep_dur);

        ticks_start = SDL_GetTicks();
        tetris_handle_inputs(frame_num);
        tetris_update(frame_num, ms_per_frame); // TODO: calculate accurate dt_ms
        //struct timespec begin_ts = gettime();
        //if (frame_num % (int)(FPS/10.0) == 0)
        tetris_render(ren);
        //struct timespec end_ts = gettime();
        //long long int dt_ns = diff_ns(end_ts, begin_ts);
        //printf("dt_ms: %ld\n", (long) (dt_ns/1000000.0));
        ticks_end = SDL_GetTicks();

        used_time = (ticks_end - ticks_start);
        dropped = (used_time > ms_per_frame) ? 1 : 0;
        sleep_dur = dropped ? 0: ms_per_frame - used_time;
        SDL_Delay(sleep_dur);

        if (dropped) {
            num_dropped++;
            printf("Dropped frames: %u\n", num_dropped);
        }

        frame_num++;
        if (frame_num % FPS == 0) {
            // 
        }
    }}

int
main(int argc, char * argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    init_program();

    tetris_init_game();
    //tetris_replay_game("inputs.dat");

    loop();

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    SDL_Quit();
    return 0;
}
