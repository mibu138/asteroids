#ifndef W_WORLD_H
#define W_WORLD_H

#include "m_math.h"
#include "z_memory.h"

#define W_MAX_OBJ 64

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
    float angVel;
    float mass;
    Geo*  geo;
} W_Object;

extern int      w_ObjectCount;
extern W_Object w_Objects[W_MAX_OBJ];
extern Geo      w_Geos[W_MAX_OBJ];
extern Z_block* w_VertexBlock;
extern Vertex*  w_VertexBuffer;

void w_Init(void);
void w_Update(void);
void w_CleanUp(void);

#endif /* end of include guard: W_WORLD_H */
