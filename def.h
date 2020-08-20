#ifndef DEF_H
#define DEF_H

#include <stdio.h>
#include <setjmp.h>

#ifndef VERBOSE
#define VERBOSE 1
#endif

#if VERBOSE > 0
#define V1_PRINT(msg, args...) printf(msg, ## args)
#else
#define V1_PRINT(msg, args...)
#endif

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000

// key values are ascii lower case
#define KEY_W     119
#define KEY_A     97
#define KEY_S     115
#define KEY_D     100
#define KEY_SPACE 32
#define KEY_ESC   27

extern jmp_buf exit_game;

#endif /* end of include guard: DEF_H */
