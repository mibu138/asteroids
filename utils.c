#include "utils.h"
#include <stdio.h>

void printVec2(const Vec2* vec)
{
    printf("x: %f, y: %f\n", vec->x, vec->y);
}

static void printGeo(const Geo* geo, const Vertex* buffer)
{
    printf("Vertex count: %d\n", geo->vertCount);
    for (int i = 0; i < geo->vertCount; i++) 
    {
        printf("Vertex %d: ", i);
        printVec2(buffer + geo->vertIndex + i);
    }
}

void printWorld(const World* world)
{
    printf("World info:\n");
    printf("World has %d active objects.\n", world->objectCount);
    for (int i = 0; i < world->objectCount; i++) 
    {
        const W_object* obj = &world->objects[i];
        printf("Object %d:\n", i);
        printf("Pos:   ");
        printVec2(&obj->pos);
        printf("Vel:   ");
        printVec2(&obj->vel);
        printf("Accel: ");
        printVec2(&obj->accel);
        printf("Angle: %f\n", obj->angle);
        printf("Mass:  %f\n", obj->mass);
        printf("Geo:   ");
        printGeo(obj->geo, world->vertexBuffer);
    }
}
