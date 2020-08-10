#include "g_game.h"
#include "r_render.h"
#include "d_display.h"
#include "def.h"
#include <assert.h>

Player player;

static void initFrameCommands(void)
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
    vkCmdEndRenderPass(frame->commandBuffer);

    r = vkEndCommandBuffer(frame->commandBuffer);
    assert ( VK_SUCCESS == r );

    r_PresentFrame();

    frame = r_RequestFrame();

    vkBeginCommandBuffer(frame->commandBuffer, &cbbi);

    rpassInfo.framebuffer = frame->frameBuffer;

    vkCmdBeginRenderPass(frame->commandBuffer, &rpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(frame->commandBuffer);

    r = vkEndCommandBuffer(frame->commandBuffer);
    assert ( VK_SUCCESS == r );

    r_PresentFrame();
}

void initPlayer(void)
{
    player.object = &world.objects[0];
}

void g_Init(void)
{
    initFrameCommands();
    initPlayer();
}

void g_Destroy(void)
{
}

