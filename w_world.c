#include "w_world.h"
#include "z_memory.h"
#include <stdlib.h>
#include <assert.h>

#define MAX_VERTS_PER_OBJ 16

World world;
Geo   geos[W_MAX_OBJ];
Z_block* vertexBlock;
Vertex*  vertexBuffer;

void w_Init(void)
{
    world.objectCount = 6;
    vertexBlock = z_RequestBlock(MAX_VERTS_PER_OBJ * W_MAX_OBJ);
    vertexBuffer = (Vertex*)vertexBlock->address;
    world.vertexBuffer = vertexBuffer;
    assert(world.objectCount <= W_MAX_OBJ);
    for (int i = 0; i < world.objectCount; i++) 
    {
        world.objects[i].accel = (Vec2){0.0, 0.0};
        world.objects[i].vel   = (Vec2){0.0, 0.0};
        world.objects[i].pos   = (Vec2){0.5, 0.5};
        world.objects[i].mass  = 1.0;
        world.objects[i].angle = 0.0;
        geos[i].vertIndex = i * MAX_VERTS_PER_OBJ;

        Vertex* verts = vertexBuffer + geos[i].vertIndex;
        int     vertCount;

        if (i == 0) //is player
        {
            vertCount= 3;
            verts[0] = (Vec2){0.2, 0.2};
            verts[1] = (Vec2){-0.2, 0.2};
            verts[2] = (Vec2){0.0, -0.2};
        }
        else
        {
            vertCount = 4;
            float r = 0.3 + i * 0.05;
            verts[0] = (Vec2){r, r};
            verts[1] = (Vec2){-r, r};
            verts[2] = (Vec2){-r, -r};
            verts[3] = (Vec2){r, -r};
        }
        geos[i].vertCount    = vertCount;
        world.objects[i].geo = &geos[i];
    }
}

void w_CleanUp()
{
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
