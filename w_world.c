#include "w_world.h"
#include "m_math.h"
#include "w_create.h"
#include "v_memory.h"
#include "r_render.h"
#include "w_collision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int        w_ObjectCount;
int        w_EmitableCount;
W_Object   w_Objects[W_MAX_OBJ];
W_Emitable w_Emitables[W_MAX_EMIT];
Geo        w_Geos[W_MAX_OBJ];
V_block*   w_ObjectVertexBlock;
V_block*   w_ObjectIndexBlock;
V_block*   w_EmitableVertexBlock;
Vertex*    w_ObjectVertexBuffer;
Index*     w_ObjectIndexBuffer;
Vertex*    w_EmitableVertexBuffer;

Collider w_Colliders[W_MAX_OBJ];

static void rotateGeo(const float angle, const Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Rotate(angle, &w_ObjectVertexBuffer[geo->vertIndex + i]);
    }
}

static void translateGeo(const Vec2 t, const Geo* geo)
{
    for (int i = 0; i < geo->vertCount; i++) 
    {
        m_Translate(t, &w_ObjectVertexBuffer[geo->vertIndex + i]);
    }
}

static void resetObjectGeo(const int index)
{
    assert( index >= 0 );
    const W_Object* object = &w_Objects[index];
    const Vec2 t = { -1 * object->prevPos.x, -1 * object->prevPos.y };
    translateGeo(t, &w_Geos[index]);
    rotateGeo(-1 * object->prevAngle, &w_Geos[index]); // reset
}

static void updateObjectGeo(const int index)
{
    assert( index >= 0 );
    const W_Object* object = &w_Objects[index];
    rotateGeo(object->angle, &w_Geos[index]);
    translateGeo(object->pos, &w_Geos[index]);
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

static void updatePlayer(void)
{
    W_Object* player = &w_Objects[0];
    float speed = m_Length(player->vel);
    float drag;
    player->drag = (drag = speed * 5) > 1 ? 1 : drag;
}

static void updateObject(const int index)
{
    assert( index >= 0 );
    W_Object* object = &w_Objects[index];
    object->prevAngle = object->angle;
    object->prevPos   = object->pos;
    m_Add(object->accel, &object->vel);
    m_Add(object->vel, &object->pos);
    wrapAround(object);
    m_Scale(1 - object->drag, &object->vel);
    object->angVel += object->angAccel;
    object->angle += object->angVel;
    object->angVel *= (1 - object->angDrag);
}

static void updateEmitable(const int index)
{
    assert( index >= 0 );
    W_Emitable* emitable = &w_Emitables[index];
    emitable->prevPos = emitable->pos;
    m_Add(emitable->vel, &emitable->pos);
}

static void initObjects(void)
{
    w_ObjectCount = 6;
    w_ObjectVertexBlock = v_RequestBlock(MAX_VERTS_PER_OBJ * W_MAX_OBJ * sizeof(Vertex));
    w_ObjectVertexBuffer = (Vertex*)w_ObjectVertexBlock->address;
    //w_ObjectIndexBlock  = z_RequestBlock(
    assert(w_ObjectCount <= W_MAX_OBJ);
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        w_Geos[i].vertCount = MAX_VERTS_PER_OBJ;
        w_Geos[i].vertIndex = i * MAX_VERTS_PER_OBJ;
    }
    for (int i = 0; i < w_ObjectCount; i++) 
    {
        float angVel =  0.01 * m_Rand();
        float tx = m_RandNeg();
        float ty = m_RandNeg();
        w_Objects[i].pos   = (Vec2){tx, ty};
        w_Objects[i].mass  = 1.0;
        w_Objects[i].angle = i;
        w_Objects[i].stage = BIG;

        if (i == 0) //is player
        {
            w_GeneratePlayerShip(i);
            angVel = 0.0;
            w_Objects[i].drag = 0.4;
            w_Objects[i].angDrag = 0.1;
        }
        else
        {
            w_GenerateAsteroidRand1(i, 0.1);
            w_Objects[i].vel.x = m_RandNeg() * INIT_SPEED;
            w_Objects[i].vel.y = m_RandNeg() * INIT_SPEED;
        }

        w_Objects[i].angVel = angVel;

        updateObjectGeo(i);
    }
}

static void initEmitables(void)
{
    w_EmitableVertexBlock = v_RequestBlock(W_MAX_EMIT * sizeof(Vertex));
    w_EmitableVertexBuffer = (Vertex*)w_EmitableVertexBlock->address;
}

static void swapEmit(int a, int b)
{
    assert( a >= 0 && b >= 0 );
    W_Emitable temp = w_Emitables[a];
    w_Emitables[a] = w_Emitables[b];
    w_Emitables[b] = temp;
    Vertex tempVert = w_EmitableVertexBuffer[a];
    w_EmitableVertexBuffer[a] = w_EmitableVertexBuffer[b];
    w_EmitableVertexBuffer[b] = tempVert;
}

static void swapObject(int a, int b)
{
    assert( a >= 0 && b >= 0 );
    const W_Object tempObject = w_Objects[a];
    w_Objects[a] = w_Objects[b];
    w_Objects[b] = tempObject;
    const Geo tempGeo = w_Geos[a];
    w_Geos[a] = w_Geos[b];
    w_Geos[b] = tempGeo;
    const Collider tempCol = w_Colliders[a];
    w_Colliders[a] = w_Colliders[b];
    w_Colliders[b] = tempCol;
}

static void physicsUpdate(void)
{
    const int objectCount = w_ObjectCount;
    const int emitableCount = w_EmitableCount;
    for (int i = 0; i < objectCount; i++) 
    {
        updateObject(i);
    }
    for (int i = 0; i < emitableCount; i++) 
    {
        updateEmitable(i);
    }
}

static void collisionsUpdate(void)
{
    w_DetectCollisions();
}

static void initAsteroid(W_Object* obj)
{
    // only need to explicitly set what should be non-zero
    W_Object newObj = {
        .mass = 1.0,
        .destroyed = false,
    };
    *obj = newObj;
}

static void spawnChildren()
{
    const int childCount = 4;
    const int objCount = w_ObjectCount;
    assert(childCount + objCount < W_MAX_OBJ);
    const Vec2 basePos = w_Objects[objCount].pos;
    const Vec2 baseVel = w_Objects[objCount].vel;
    const AstStage baseStage = w_Objects[objCount].stage;
    const float baseAngVel = w_Objects[objCount].angVel;
    const float baseRadius = w_Colliders[objCount].radius / 4;
    for (int i = objCount; i < objCount + childCount; i++) 
    {
        W_Object* obj = &w_Objects[i];          
        initAsteroid(obj);
        obj->angle = m_Rand() * M_PI * 2;
        obj->stage = baseStage + 1;
        Vec2 vel = {INIT_SPEED / 2, 0};
        m_Rotate(i, &vel);
        //m_Add(baseVel, &vel);
        obj->vel = vel;
        obj->pos = basePos;
        obj->angVel = baseAngVel;
        w_GenerateAsteroidRand1(i, baseRadius);
    }
    w_ObjectCount += childCount;
}

static void reapAndSpawn(void)
{
    const int objCount = w_ObjectCount;
    const int emtCount = w_EmitableCount;
    int deadObject = -1;
    for (int i = 0; i < objCount; i++) 
    {
        if (w_Objects[i].destroyed)
        {
            //destroyObject(i);
            deadObject = i;
            break;
        }
    }
    if (deadObject != -1)
    {
        swapObject(deadObject, --w_ObjectCount);
        assert(w_ObjectCount >= 0);
        if (w_Objects[w_ObjectCount].stage < FINAL)
            spawnChildren();
    }
    int deadEmit = -1;
    for (int i = 0; i < emtCount; i++) 
    {
        W_Emitable* emitable = &w_Emitables[i];
        emitable->lifeTicks--;
        if (w_Emitables[i].lifeTicks <= 0)
        {
            deadEmit = i;
        }
    }
    if (deadEmit != -1)
    {
        swapEmit(deadEmit, --w_EmitableCount);
        assert( w_EmitableCount >= 0 );
    }
}

void w_DetectCollisions(void)
{
    HitInfo hi = w_DetectBulletObjectCols();
    if (hi.collision)
    {
        w_Emitables[hi.object1].lifeTicks = 0;
        w_Objects[hi.object2].destroyed = true;
    }
}

void w_Init(void)
{
    initObjects();
    initEmitables();
}

void w_Update(void)
{
    updatePlayer();
    physicsUpdate();
    collisionsUpdate();
    r_WaitOnQueueSubmit();
    reapAndSpawn();
    w_UpdateDrawables();
}

void w_UpdateDrawables(void)
{
    const int objCount = w_ObjectCount;
    const int emtCount = w_EmitableCount;
    for (int i = 0; i < objCount; i++) 
    {
        resetObjectGeo(i);
        updateObjectGeo(i);
    }
    for (int i = 0; i < emtCount; i++) 
    {
        Vertex* vert = &w_EmitableVertexBuffer[i];
        vert->x = 0.0;
        vert->y = 0.0;
        m_Translate(w_Emitables[i].pos, vert);
    }
}

void w_CleanUp()
{
    for (int i = 0; i < W_MAX_OBJ; i++) 
    {
        // nothing for now
    }
}
