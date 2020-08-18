#include "w_world.h"
#include "m_math.h"
#include "z_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_VERTS_PER_OBJ 16

int        w_ObjectCount;
int        w_CurEmitable;
W_Object   w_Objects[W_MAX_OBJ];
W_Emitable w_Emitables[W_MAX_EMIT];
Geo        w_Geos[W_MAX_OBJ];
Z_block*   w_ObjectVertexBlock;
Z_block*   w_EmitableVertexBlock;
Vertex*    w_ObjectVertexBuffer;
Vertex*    w_EmitableVertexBuffer;

static void rotateGeo(const float angle, Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Rotate(angle, &w_ObjectVertexBuffer[geo->vertIndex + i]);
    }
}

static void translateGeo(const Vec2 t, Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Translate(t, &w_ObjectVertexBuffer[geo->vertIndex + i]);
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

static void wrapAround(W_Object* object)
{
    float x = object->pos.x;
    float y = object->pos.y;
    if (x < -1) x += 2;
    if (x >  1) x -= 2;
    if (y < -1) y += 2;
    if (y >  1) y -= 2;
    object->pos.x = x;
    object->pos.y = y;
}

static void updateObject(W_Object* object)
{
    resetObjectGeo(object);
    m_Add(object->accel, &object->vel);
    m_Add(object->vel, &object->pos);
    wrapAround(object);
    m_Scale(1 - object->drag, &object->vel);
    object->angVel += object->angAccel;
    object->angle += object->angVel;
    object->angVel *= (1 - object->angDrag);
    updateObjectGeo(object);
}

static void updateEmitable(W_Emitable* emitable)
{
    emitable->vert->x = 0.0;
    emitable->vert->y = 0.0;
    m_Add(emitable->vel, &emitable->pos);
    m_Translate(emitable->pos, emitable->vert);
    emitable->lifeTicks--;
    if (emitable->lifeTicks == 0)
    {
        // is dead
        emitable->pos.x = -5; 
        emitable->pos.y =  0;
    }
}

static void initObjects(void)
{
    w_ObjectCount = 10;
    w_ObjectVertexBlock = z_RequestBlock(MAX_VERTS_PER_OBJ * W_MAX_OBJ * sizeof(Vertex));
    w_ObjectVertexBuffer = (Vertex*)w_ObjectVertexBlock->address;
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

        Vertex* verts = w_ObjectVertexBuffer + w_Geos[i].vertIndex;
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
            w_Objects[i].drag = 0.4;
            w_Objects[i].angDrag = 0.1;
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

static void initEmitables(void)
{
    w_EmitableVertexBlock = z_RequestBlock(W_MAX_EMIT * sizeof(Vertex));
    w_EmitableVertexBuffer = (Vertex*)w_EmitableVertexBlock->address;
    for (int i = 0; i < W_MAX_EMIT; i++) 
    {
        w_Emitables[i].vert = &w_EmitableVertexBuffer[i];
        w_Emitables[i].vert->x = -5; // just get them off screen
    }
}

void w_Init(void)
{
    initObjects();
    initEmitables();
}

void w_Update(void)
{
    for (int i = 0; i < w_ObjectCount; i++) 
    {
        updateObject(&w_Objects[i]);
    }
    for (int i = 0; i < W_MAX_EMIT; i++) 
    {
        if (w_Emitables[i].lifeTicks)
        {
            updateEmitable(&w_Emitables[i]);
        }
    }
}

void w_CleanUp()
{
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
