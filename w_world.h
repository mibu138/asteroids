#ifndef W_WORLD_H
#define W_WORLD_H

#include "m_math.h"
#include "z_memory.h"

#define W_MAX_OBJ  64
#define W_MAX_EMIT 32

typedef Vec2 Vertex;

typedef struct {
    int      vertCount;
    int      vertIndex; // index to the first vertex
} Geo;

typedef struct {
    Vec2  pos;
    Vec2  vel;
    Vec2  accel;
    float drag;
    float angle;
    float angVel;
    float angAccel;
    float angDrag;
    float mass;
    Geo*  geo;
} W_Object;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    int  lifeTicks;
} W_Emitable;

extern int        w_ObjectCount;
extern int        w_ActiveEmit;
extern W_Object   w_Objects[W_MAX_OBJ];
extern W_Emitable w_Emitables[W_MAX_EMIT];
extern Geo        w_Geos[W_MAX_OBJ];
extern Z_block*   w_ObjectVertexBlock;
extern Z_block*   w_EmitableVertexBlock;
extern Vertex*    w_ObjectVertexBuffer;
extern Vertex*    w_EmitableVertexBuffer;

void w_Init(void);
void w_Update(void);
void w_CleanUp(void);

#endif /* end of include guard: W_WORLD_H */
