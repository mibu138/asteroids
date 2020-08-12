#ifndef W_WORLD_H
#define W_WORLD_H

#include "m_math.h"

#define MAX_OBJ 4

typedef struct geo {
    int   pointCount;
    Vec2* points;
} Geo;

typedef struct w_object {
    Vec2  pos;
    Vec2  vel;
    Vec2  accel;
    float angle;
    float mass;
    Geo*  geo;
} W_object;

typedef struct world {
    W_object objects[MAX_OBJ];
    int      objectCount;
} World;

extern World world;
extern Geo   geos[MAX_OBJ];

void w_Init(void);
void w_CleanUp(void);

#endif /* end of include guard: W_WORLD_H */
