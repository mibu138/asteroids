#ifndef G_GAME_H
#define G_GAME_H

#include "w_world.h"
#include "i_input.h"

typedef struct player {
    W_Object* object;
} Player;

void g_Init(void);
void g_Responder(const I_Event* event);
void g_Update(void);
void g_RotateAllObjects(void);
void g_CleanUp(void);

#endif /* end of include guard: G_GAME_H */
