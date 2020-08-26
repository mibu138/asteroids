#include "m_math.h"
#include "g_game.h"
#include "r_pipeline.h"
#include "def.h"
#include <assert.h>
#include <setjmp.h>
#include <vulkan/vulkan_core.h>

Player player;

static const Vec2 frontDir = {0.0, -1.0};

static bool moveForward;
static bool turnLeft;
static bool turnRight;
static bool fire;

// should make a seperate r_* module for placing game world specific rendering
// stuff in


void initPlayer(void)
{
    player.object = &w_Objects[0];
}

void g_Init(void)
{
    initPlayer();
}

void g_Responder(const I_Event* event)
{
    switch (event->type) 
    {
        case i_Keydown: switch (event->data)
        {
            case KEY_W: moveForward = true; break;
            case KEY_A: turnLeft = true; break;
            case KEY_D: turnRight = true; break;
            case KEY_SPACE: fire = true; break;
            case KEY_ESC: longjmp(exit_game, 1);
            default: return;
        } break;
        case i_Keyup:   switch (event->data)
        {
            case KEY_W: moveForward = false; break;
            case KEY_A: turnLeft = false; break;
            case KEY_D: turnRight = false; break;
            case KEY_SPACE: fire = false; break;
            default: return;
        } break;
        default: assert(0); // error
    }
}

void g_Update(void)
{
    if (moveForward)
    {
        Vec2 accel = frontDir;
        m_Scale(0.002, &accel);
        m_Rotate(player.object->angle, &accel);
        player.object->accel = accel;
        //m_Add(accel, &player.object->accel);
    }
    else 
    {
        player.object->accel.x = 0.0;
        player.object->accel.y = 0.0;
    }

    if ((turnLeft && turnRight) || (!turnLeft && !turnRight))
    {
        player.object->angAccel = 0.0;
    }
    else if (turnLeft)
    {
        player.object->angAccel = -0.01;
    }
    else if (turnRight)
    {
        player.object->angAccel =  0.01; 
    }

    if (fire)
    {
        assert ( w_EmitableCount < W_MAX_EMIT );
        W_Emitable* beam = &w_Emitables[w_EmitableCount++];
        beam->lifeTicks = 100;
        beam->pos = player.object->pos;
        Vec2 dir = frontDir;
        m_Scale(0.02, &dir);
        m_Rotate(player.object->angle, &dir);
        beam->vel = dir;
        fire = false;
    }
}

void g_CleanUp(void)
{
}

