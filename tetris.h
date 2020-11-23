#ifndef TETRIS_H
#define TETRIS_H

#if OSX
#  include <SDL2/SDL.h>
#else
#  include <SDL.h>
#endif

#include <stdint.h>

void tetris_init_game();
void tetris_replay_game(const char * filename);
void tetris_handle_inputs(uint32_t frame_num);
void tetris_update(uint32_t frame_num, uint32_t dt_ms);
void tetris_render(SDL_Renderer * ren);

#endif /* TETRIS_H */
