#include "w_world.h"
#include <stdlib.h>
#include <assert.h>

World world;
Geo   geos[MAX_OBJ];

void w_Init(void)
{
    world.objectCount = 1;
    assert(world.objectCount < MAX_OBJ);
    for (int i = 0; i < world.objectCount; i++) 
    {
        world.objects[i].accel = (Vec2){0.0, 0.0};
        world.objects[i].vel   = (Vec2){0.0, 0.0};
        world.objects[i].pos   = (Vec2){0.5, 0.5};
        world.objects[i].mass  = 1.0;
        world.objects[i].angle = 0.0;
        Vec2* points;
        int   pointCount;
        if (i == 0) //is player
        {
            pointCount = 3;
            points = malloc(sizeof(Vec2) * pointCount);
            assert(points);
            points[0] = (Vec2){0.2, 0.2};
            points[1] = (Vec2){-0.2, 0.2};
            points[2] = (Vec2){0.0, -0.2};
        }
        else
        {
            pointCount = 4;
            points = malloc(sizeof(Vec2) * pointCount); //just making squares now
            assert(points);
            points[0] = (Vec2){0.2, 0.2};
            points[1] = (Vec2){-0.2, 0.2};
            points[2] = (Vec2){-0.2, -0.2};
            points[3] = (Vec2){0.2, -0.2};
        }
        geos[i].points = points;
        geos[i].pointCount = pointCount;
        world.objects[i].geo = &geos[i];
    }
}

void w_Destroy()
{
    for (int i = 0; i < MAX_OBJ; i++) 
    {
        if (geos[i].points)
            free(geos[i].points);
    }
}
