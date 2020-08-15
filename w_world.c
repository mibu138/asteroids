#include "w_world.h"
#include "m_math.h"
#include "z_memory.h"
#include <stdlib.h>
#include <assert.h>

#define MAX_VERTS_PER_OBJ 16

int      w_ObjectCount;
W_Object w_Objects[W_MAX_OBJ];
Geo      w_Geos[W_MAX_OBJ];
Z_block* w_VertexBlock;
Vertex*  w_VertexBuffer;

static void rotateGeo(const float angle, Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Rotate(angle, &w_VertexBuffer[geo->vertIndex + i]);
    }
}

void w_Init(void)
{
    w_ObjectCount = 2;
    w_VertexBlock = z_RequestBlock(MAX_VERTS_PER_OBJ * W_MAX_OBJ);
    w_VertexBuffer = (Vertex*)w_VertexBlock->address;
    w_VertexBuffer = w_VertexBuffer;
    assert(w_ObjectCount <= W_MAX_OBJ);
    for (int i = 0; i < w_ObjectCount; i++) 
    {
        w_Objects[i].accel = (Vec2){0.0, 0.0};
        w_Objects[i].vel   = (Vec2){0.0, 0.0};
        w_Objects[i].pos   = (Vec2){0.5, 0.5};
        w_Objects[i].mass  = 1.0;
        w_Objects[i].angle = 0.0;
        w_Geos[i].vertIndex = i * MAX_VERTS_PER_OBJ;

        Vertex* verts = w_VertexBuffer + w_Geos[i].vertIndex;
        int     vertCount;

        if (i == 0) //is player
        {
            vertCount= 4;
            verts[0] = (Vec2){0.2, 0.2};
            verts[1] = (Vec2){-0.2, 0.2};
            verts[2] = (Vec2){0.0, -0.2};
            verts[3] = (Vec2){0.2, 0.2};
        }
        else
        {
            vertCount = 5;
            float r = 0.1;
            verts[0] = (Vec2){r, r};
            verts[1] = (Vec2){-r, r};
            verts[2] = (Vec2){-r, -r};
            verts[3] = (Vec2){r, -r};
            verts[4] = (Vec2){r, r};
        }
        w_Geos[i].vertCount    = vertCount;
        w_Objects[i].geo = &w_Geos[i];

        rotateGeo(i * 0.1, &w_Geos[i]);
    }
}

void w_CleanUp()
{
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
