#include "m_math.h"
#include "g_game.h"
#include "r_render.h"
#include "r_pipeline.h"
#include "def.h"
#include <assert.h>
#include <vulkan/vulkan_core.h>

Player player;

static const Vec2 frontDir = {0.0, -1.0};

static bool moveForward;
static bool turnLeft;
static bool turnRight;
static bool fire;

static void initFrameCommands(void)
{
    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        Frame* frame = r_RequestFrame();

        VkResult r;
        VkCommandBufferBeginInfo cbbi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        r = vkBeginCommandBuffer(frame->commandBuffer, &cbbi);

        VkClearValue clearValue = {0.005f, 0.0f, 0.01f, 1.0f};

        VkRenderPassBeginInfo rpassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .clearValueCount = 1,
            .pClearValues = &clearValue,
            .renderArea = {{0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}},
            .renderPass = *frame->pRenderPass,
            .framebuffer = frame->frameBuffer,
        };

        vkCmdBeginRenderPass(frame->commandBuffer, &rpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[r_ObjPipeId]);

        VkDeviceSize vertBufferOffset = w_ObjectVertexBlock->vOffset; 

        assert( 0 == vertBufferOffset ); // should be the offset of the very first vert in the buffer
                                        // which happens to be 0 right now (though does not have to be)
        vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, w_ObjectVertexBlock->vBuffer, &vertBufferOffset);

        for (int i = 0; i < W_MAX_OBJ; i++) 
        {
            vkCmdDraw(frame->commandBuffer, w_Geos[i].vertCount, 1, w_Geos[i].vertIndex, 0);
        }

        vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[r_EmitPipeId]);

        VkDeviceSize emitVertBufferOffset = w_EmitableVertexBlock->vOffset;

        vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, w_EmitableVertexBlock->vBuffer, &emitVertBufferOffset);

        vkCmdDraw(frame->commandBuffer, W_MAX_EMIT, 1, 0, 0);

        vkCmdEndRenderPass(frame->commandBuffer);

        r = vkEndCommandBuffer(frame->commandBuffer);
        assert ( VK_SUCCESS == r );

        r_PresentFrame();
    }
}

void initPlayer(void)
{
    player.object = &w_Objects[0];
}

void g_Init(void)
{
    initFrameCommands();
    initPlayer();
}

void g_Responder(const I_Event* event)
{
    if (event->type == i_Keydown) 
    {
        switch (event->data) 
        {
            case KEY_W: moveForward = true; break;
            case KEY_A: turnLeft = true; break;
            case KEY_D: turnRight = true; break;
            case KEY_SPACE: fire = true; break;
            default: return;
        }
    }
    if (event->type == i_Keyup)
    {
        switch (event->data) 
        {
            case KEY_W: moveForward = false; break;
            case KEY_A: turnLeft = false; break;
            case KEY_D: turnRight = false; break;
            case KEY_SPACE: fire = false; break;
            default: return;
        }
    }
}

void g_Update(void)
{
    if (moveForward)
    {
        Vec2 accel = frontDir;
        m_Scale(0.01, &accel);
        m_Rotate(player.object->angle, &accel);
        player.object->accel = accel;
        //m_Add(accel, &player.object->accel);
        printf("Accelerating...\n");
    }
    else 
    {
        player.object->accel.x = 0.0;
        player.object->accel.y = 0.0;
    }
    if (turnLeft)
    {
        player.object->angAccel = -0.01;
    }
    if (turnRight)
    {
        player.object->angAccel =  0.01; 
    }
    if (!turnRight && !turnLeft)
        player.object->angAccel = 0.0;

    if (fire)
    {
        printf("FIRE!\n");
        W_Emitable* beam = &w_Emitables[w_CurEmitable];
        beam->lifeTicks = 100;
        beam->pos = player.object->pos;
        Vec2 dir = frontDir;
        m_Scale(0.03, &dir);
        m_Rotate(player.object->angle, &dir);
        beam->vel = dir;
        w_CurEmitable = (w_CurEmitable + 1) % W_MAX_EMIT;
        fire = false;
    }
}

void g_CleanUp(void)
{
}

