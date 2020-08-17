#include "w_world.h"
#include "m_math.h"
#include "z_memory.h"
#include <stdio.h>
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

static void translateGeo(const Vec2 t, Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Translate(t, &w_VertexBuffer[geo->vertIndex + i]);
    }
}

static void resetObjectGeo(W_Object* object)
{
    const Vec2 t = { -1 * object->pos.x, -1 * object->pos.y };
    translateGeo(t, object->geo);
    rotateGeo(-1 * object->angle, object->geo); // reset
}

static void updateObjectGeo(W_Object* object)
{
    rotateGeo(object->angle, object->geo);
    translateGeo(object->pos, object->geo);
}

static void updateObject(W_Object* object)
{
    resetObjectGeo(object);
    m_Add(object->accel, &object->vel);
    m_Add(object->vel, &object->pos);
    m_Scale(0.5, &object->accel);
    m_Scale(0.5, &object->vel);
    object->angVel += object->angAccel;
    object->angle += object->angVel;
    object->angVel *= 0.9;
    updateObjectGeo(object);
}

void w_Init(void)
{
    w_ObjectCount = 10;
    w_VertexBlock = z_RequestBlock(MAX_VERTS_PER_OBJ * W_MAX_OBJ * sizeof(Vertex));
    w_VertexBuffer = (Vertex*)w_VertexBlock->address;
    w_VertexBuffer = w_VertexBuffer;
    assert(w_ObjectCount <= W_MAX_OBJ);
    for (int i = 0; i < w_ObjectCount; i++) 
    {
        float angVel =  0.01 * m_Rand();
        float tx = m_RandNeg();
        float ty = m_RandNeg();
        w_Objects[i].pos   = (Vec2){tx, ty};
        w_Objects[i].mass  = 1.0;
        w_Objects[i].angle = i;
        w_Geos[i].vertIndex = i * MAX_VERTS_PER_OBJ;

        Vertex* verts = w_VertexBuffer + w_Geos[i].vertIndex;
        int     vertCount;

        if (i == 0) //is player
        {
            vertCount= 4;
            float r = 0.075;
            verts[0] = (Vec2){0.0, -r};
            verts[1] = (Vec2){-r/2, r/2};
            verts[2] = (Vec2){r/2, r/2};
            verts[3] = (Vec2){0.0, -r};
            angVel = 0.0;
        }
        else
        {
            vertCount = 5;
            float r = 0.1 + i * .01;
            verts[0] = (Vec2){r, r};
            verts[1] = (Vec2){-r, r};
            verts[2] = (Vec2){-r, -r};
            verts[3] = (Vec2){r, -r};
            verts[4] = (Vec2){r, r};
        }

        w_Objects[i].angVel = angVel;
        w_Geos[i].vertCount    = vertCount;
        w_Objects[i].geo = &w_Geos[i];

        updateObjectGeo(&w_Objects[i]);
    }
}

void w_Update(void)
{
    for (int i = 0; i < w_ObjectCount; i++) 
    {
        updateObject(&w_Objects[i]);
    }
}

void w_CleanUp()
{
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
