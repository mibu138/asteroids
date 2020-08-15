#ifndef W_WORLD_H
#define W_WORLD_H

#include "m_math.h"
#include "z_memory.h"

#define W_MAX_OBJ 16

typedef Vec2 Vertex;

typedef struct geo {
    int      vertCount;
    int      vertIndex; // index to the first vertex
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
    W_object objects[W_MAX_OBJ];
    int      objectCount;
    Vertex*  vertexBuffer;
} World;

extern World    world;
extern Geo      geos[W_MAX_OBJ];
extern Z_block* vertexBlock;
extern Vertex*  vertexBuffer;

void w_Init(void);
void w_CleanUp(void);

#endif /* end of include guard: W_WORLD_H */
