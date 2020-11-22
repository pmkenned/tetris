#ifndef MAIN_H
#define MAIN_H

extern int done;
extern int win_w;
extern int win_h;
extern int sq;

extern int use_seed;
extern int seed;

enum {
    FPS = 30,
};

void toggle_fullscreen();

#endif /* MAIN_H */
