#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "main.h"
#include "tetris.h"
#include "piece.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

static int replay_game = 0;
static int game_over = 0;
static int level = 1;
static int score = 0;
static int lines_cleared = 0;

static int curr_piece_idx = 0;
static int curr_piece_ori = 0;
static int curr_piece_row = 0;
static int curr_piece_col = 0;
static int curr_piece_timer = 0;

typedef struct {
    uint32_t frame_num;
    uint32_t input;
} INPUT_T;

enum { INIT_INPUTS_CAP = 100 };

static INPUT_T * inputs = NULL;
static size_t num_inputs = 0;
static size_t inputs_cap = INIT_INPUTS_CAP;
static size_t replay_idx = 0;

static void
append_input(INPUT_T input)
{
    inputs[num_inputs++] = input;
    if (num_inputs >= inputs_cap) {
        inputs_cap *= 2;
        inputs = realloc(inputs, sizeof(*inputs) * inputs_cap);
    }
}

static void
end_game()
{
    size_t i;
    if (!replay_game) {
        FILE * fp = fopen("inputs.dat", "w");
        if (fp == NULL) {
            perror("error");
            exit(EXIT_FAILURE);
        }
        for (i=0; i<num_inputs; i++)
            fprintf(fp, "%u %u\n", inputs[i].frame_num, inputs[i].input);
        fclose(fp);
    }
    done = 1;
    free(inputs);
}

typedef struct {
    int r, g, b, a;
} COLOR_T;

static const COLOR_T colors[] = {
    { 0x20, 0x20, 0x20, SDL_ALPHA_OPAQUE },
    { 0xff, 0x00, 0xff, SDL_ALPHA_OPAQUE },
    { 0xff, 0x80, 0x00, SDL_ALPHA_OPAQUE },
    { 0xff, 0xff, 0x00, SDL_ALPHA_OPAQUE },
    { 0x00, 0x00, 0xff, SDL_ALPHA_OPAQUE },
    { 0x00, 0xff, 0xff, SDL_ALPHA_OPAQUE },
    { 0x00, 0xff, 0x00, SDL_ALPHA_OPAQUE },
    { 0xff, 0x00, 0x00, SDL_ALPHA_OPAQUE }
};

enum {
    BOARD_ROWS = 20,
    BOARD_COLS = 10,
    NUM_PIECES = 7,
    LINES_PER_LEVEL = 10
};

// TLOJISZ
typedef enum {
    BOARD_SQ_EMPTY = 0,
    BOARD_SQ_T,
    BOARD_SQ_L,
    BOARD_SQ_O,
    BOARD_SQ_J,
    BOARD_SQ_I,
    BOARD_SQ_S,
    BOARD_SQ_Z
} BOARD_SQ;

static BOARD_SQ board[BOARD_ROWS][BOARD_COLS];

static COLOR_T
get_color(BOARD_SQ sq_val)
{
    COLOR_T color;
    switch (sq_val) {
        case BOARD_SQ_EMPTY:    color = colors[0]; break;
        case BOARD_SQ_T:        color = colors[1]; break;
        case BOARD_SQ_L:        color = colors[2]; break;
        case BOARD_SQ_O:        color = colors[3]; break;
        case BOARD_SQ_J:        color = colors[4]; break;
        case BOARD_SQ_I:        color = colors[5]; break;
        case BOARD_SQ_S:        color = colors[6]; break;
        case BOARD_SQ_Z:        color = colors[7]; break;
        default:
            fprintf(stderr, "ERRORL get_color(): invalid input\n");
            exit(EXIT_FAILURE);
            break;
    }
    return color;
}

static int
piece_can_be_at(int piece_idx, int piece_ori, int piece_row, int piece_col)
{
    int r, c;
    for (r=0; r<4; r++) {
        for (c=0; c<4; c++) {
            int sq_val = pieces[piece_idx][piece_ori][r][c];
            int sq_row = piece_row + r;
            int sq_col = piece_col + c;
            if (sq_val == 0) {
                continue;
            }
            if ((sq_row >= BOARD_ROWS) || (sq_col < 0) || (sq_col >= BOARD_COLS))
                return 0;
            if (board[sq_row][sq_col] != BOARD_SQ_EMPTY)
                return 0;
        }
    }
    return 1;
}

static void
add_curr_piece_to_board()
{
    int r, c;

    for (r=0; r<4; r++) {
        for (c=0; c<4; c++) {
            int sq_val = pieces[curr_piece_idx][curr_piece_ori][r][c];
            if (sq_val == 0) {
                continue;
            }
            int sq_row = curr_piece_row + r;
            int sq_col = curr_piece_col + c;
            board[sq_row][sq_col] = curr_piece_idx + 1; // TODO: hacky
        }
    }
}

static uint32_t
frames_per_fall()
{
    return FPS/(2*level);
}

static void
init_next_piece()
{
    static int first = 1;
    static int r = 0;
    if (first) {
        r = use_seed ? seed : time(NULL);
        first = 0;
    }
    srand(r); // guarantees no interference with PRNG
    r = rand();

    curr_piece_idx = r % NUM_PIECES;
    curr_piece_row = 0;
    curr_piece_col = (BOARD_COLS/2)-2;
    curr_piece_ori = 0;
    curr_piece_timer = frames_per_fall();
    if (!piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row, curr_piece_col)) {
        game_over = 1;
    }
}

static void
clear_full_rows()
{
    int r, c;
    int lc = 0;
    for (r = BOARD_ROWS-1; r > 0; r--) {
        int row_full = 1;
        for (c = 0; c < BOARD_COLS; c++) {
            if (board[r][c] == BOARD_SQ_EMPTY) {
                row_full = 0;
                break;
            }
        }
        // push rows down
        if (row_full) {
            int r2;
            // move all prior rows down 1 row
            for (r2 = r; r2 > 0; r2--) {
                for (c = 0; c < BOARD_COLS; c++) {
                    board[r2][c] = board[r2-1][c];
                }
            }
            // clear top row
            for (c = 0; c < BOARD_COLS; c++) {
                board[0][c] = BOARD_SQ_EMPTY;
            }
            lc++;
            r++; // recheck this row
        }
    }
    lines_cleared += lc;
    score += lc*lc*5;
    if ((lc > 0) && (lines_cleared % LINES_PER_LEVEL == 0)) {
        level++;
    }
    if (lc > 0) {
        printf("Score: %u Lines: %u Level: %u\n", score, lines_cleared, level);
    }
}

static void
add_clear_next()
{
    add_curr_piece_to_board();
    clear_full_rows();
    init_next_piece();
}

static void
handle_input(uint32_t frame_num, SDL_Event e)
{
    if (e.type == SDL_QUIT) {
        end_game();
    } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                end_game();
                break;
            case SDLK_r:
                if (!replay_game)
                    append_input((INPUT_T) {frame_num, SDLK_r});
                tetris_init_game();
                break;
            case SDLK_LEFT:
                if (!game_over) {
                    if (!replay_game)
                        append_input((INPUT_T) {frame_num, SDLK_LEFT});
                    if (piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row, curr_piece_col-1))
                        curr_piece_col--;
                }
                break;
            case SDLK_RIGHT:
                if (!game_over) {
                    if (!replay_game)
                        append_input((INPUT_T) {frame_num, SDLK_RIGHT});
                    if (piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row, curr_piece_col+1))
                        curr_piece_col++;
                }
                break;
            case SDLK_UP:
                if (!game_over) {
                    if (!replay_game)
                        append_input((INPUT_T) {frame_num, SDLK_UP});
                    int new_ori = (curr_piece_ori + 1) % 4;
                    if (piece_can_be_at(curr_piece_idx, new_ori, curr_piece_row, curr_piece_col))
                        curr_piece_ori = new_ori;
                }
                break;
            case SDLK_DOWN:
                if (!game_over) {
                    if (!replay_game)
                        append_input((INPUT_T) {frame_num, SDLK_DOWN});
                    if (piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row+1, curr_piece_col))
                        curr_piece_row++;
                }
                break;
            case SDLK_SPACE:
                if (!game_over) {
                    if (!replay_game)
                        append_input((INPUT_T) {frame_num, SDLK_SPACE});
                    // move the piece down as far as it will go
                    while (piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row+1, curr_piece_col))
                        curr_piece_row++;
                    add_clear_next();
                }
                break;
            case SDLK_f:
                toggle_fullscreen();
                break;
            default:
                // unhandled input
                break;
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
    }
}

void
tetris_init_game()
{
    int r, c;
    for (r=0; r<BOARD_ROWS; r++) {
        for (c=0; c<BOARD_COLS; c++) {
            board[r][c] = BOARD_SQ_EMPTY;
        }
    }
    score = 0;
    game_over = 0;
    level = 1;
    lines_cleared = 0;
    init_next_piece();

    if (inputs != NULL)
        free(inputs);
    inputs = malloc(sizeof(*inputs)*inputs_cap);
    num_inputs = 0;
}

void
tetris_replay_game(const char * filename)
{
    char buffer[1024];
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("error");
        exit(EXIT_FAILURE);
    }
    tetris_init_game();
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        uint32_t frame_num, input;
        sscanf(buffer, "%u%u", &frame_num, &input);
        //printf("%u %u\n", x, y);
        append_input((INPUT_T) {frame_num, input});
    }
    fclose(fp);
    replay_game = 1;
    //exit(EXIT_SUCCESS);
}

void
tetris_handle_inputs(uint32_t frame_num)
{
    SDL_Event e, replay_e;
    if (replay_game) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                end_game();
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    end_game();
                }
            }
        }
        replay_e.type = SDL_KEYDOWN; // TODO: currently the only input type handled
        while (inputs[replay_idx].frame_num == frame_num) {
            replay_e.key.keysym.sym = inputs[replay_idx].input;
            handle_input(frame_num, replay_e);
            replay_idx++;
            //printf("%u %u\n", replay_idx, num_inputs);
            if (replay_idx >= num_inputs) {
                end_game();
            }
        }
    } else {
        while (SDL_PollEvent(&e)) {
            handle_input(frame_num, e);
        }
    }
}

void
tetris_update(uint32_t frame_num, uint32_t dt_ms)
{
    UNUSED(frame_num);
    UNUSED(dt_ms);
    if (!game_over) {
        curr_piece_timer--;
        if (curr_piece_timer == 0) {
            // move current piece down a row if possible, otherwise add it to board
            if (piece_can_be_at(curr_piece_idx, curr_piece_ori, curr_piece_row+1, curr_piece_col)) {
                curr_piece_row = (curr_piece_row + 1) % BOARD_ROWS;
                curr_piece_timer = frames_per_fall();
            } else {
                add_clear_next();
            }
        }
    }
}

void
tetris_render(SDL_Renderer * ren)
{
    int r, c;
    COLOR_T color;
    UNUSED(color);
    UNUSED(r);
    UNUSED(c);
    get_color(BOARD_SQ_EMPTY);

    SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(ren);

    for (r=0; r<BOARD_ROWS; r++) {
        for (c=0; c<BOARD_COLS; c++) {
            BOARD_SQ sq_val = board[r][c];
            if (game_over) {
                if (sq_val == BOARD_SQ_EMPTY)
                    SDL_SetRenderDrawColor(ren, 0x10, 0x10, 0x10, SDL_ALPHA_OPAQUE);
                else
                    SDL_SetRenderDrawColor(ren, 0x30, 0x30, 0x30, SDL_ALPHA_OPAQUE);
            } else {
                color = get_color(sq_val);
                SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
            }
            SDL_Rect fillRect = { (int)(win_w/2 + (c-5)*sq), (int)(win_h/2+(r-10)*sq), sq-2, sq-2 };
            SDL_RenderFillRect(ren, &fillRect);
        }
    }

    if (game_over) {
        SDL_SetRenderDrawColor(ren, 0x30, 0x30, 0x30, SDL_ALPHA_OPAQUE);
    } else {
        color = get_color(curr_piece_idx+1); // TODO: hacky
        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    }
    for (r=0; r<4; r++) {
        for (c=0; c<4; c++) {
            int sq_val = pieces[curr_piece_idx][curr_piece_ori][r][c];
            int sq_row = curr_piece_row + r;
            int sq_col = curr_piece_col + c;
            if (sq_val == 0) {
                continue;
            }
            SDL_Rect fillRect = { (int)(win_w/2 + (sq_col-5)*sq), (int)(win_h/2+(sq_row-10)*sq), sq-2, sq-2 };
            SDL_RenderFillRect(ren, &fillRect);
        }
    }

    SDL_RenderPresent(ren);
}
