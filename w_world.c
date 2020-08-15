#include "w_world.h"
#include "z_memory.h"
#include <stdlib.h>
#include <assert.h>

World world;
Geo   geos[MAX_OBJ];

void w_Init(void)
{
    world.objectCount = 4;
    assert(world.objectCount <= MAX_OBJ);
    for (int i = 0; i < world.objectCount; i++) 
    {
        world.objects[i].accel = (Vec2){0.0, 0.0};
        world.objects[i].vel   = (Vec2){0.0, 0.0};
        world.objects[i].pos   = (Vec2){0.5, 0.5};
        world.objects[i].mass  = 1.0;
        world.objects[i].angle = 0.0;

        Z_block* pBlock;
        Vec2*    points;
        int   pointCount;

        if (i == 0) //is player
        {
            pointCount = 3;
            pBlock = z_RequestBlock(sizeof(Vec2) * pointCount);
            points = (Vec2*)pBlock->address;
            points[0] = (Vec2){0.2, 0.2};
            points[1] = (Vec2){-0.2, 0.2};
            points[2] = (Vec2){0.0, -0.2};
        }
        else
        {
            pointCount = 4;
            pBlock = z_RequestBlock(sizeof(Vec2) * pointCount);
            points = (Vec2*)pBlock->address;
            points[0] = (Vec2){0.3, 0.3};
            points[1] = (Vec2){-0.3, 0.3};
            points[2] = (Vec2){-0.3, -0.3};
            points[3] = (Vec2){0.3, -0.3};
        }
        geos[i].pPointBlock = pBlock;
        geos[i].points = points;
        geos[i].pointCount = pointCount;
        world.objects[i].geo = &geos[i];
    }
}

void w_CleanUp()
{
    for (int i = 0; i < MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
