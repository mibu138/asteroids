#include "d_display.h"
#include "r_render.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>


int main(int argc, char *argv[])
{
    d_Init();
    r_Init();

    Frame* frame = r_RequestFrame();

    VkResult r;
    VkCommandBufferBeginInfo cbbi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    r = vkBeginCommandBuffer(frame->commandBuffer, &cbbi);

    VkClearValue clearValue = {1.0f, 0.5f, 0.0f, 1.0f};

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

    sleep(1);

    frame = r_RequestFrame();

    vkBeginCommandBuffer(frame->commandBuffer, &cbbi);

    clearValue = (VkClearValue){0.0, 0.5, 1.0, 1.0};
    rpassInfo.framebuffer = frame->frameBuffer;

    vkCmdBeginRenderPass(frame->commandBuffer, &rpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(frame->commandBuffer);

    r = vkEndCommandBuffer(frame->commandBuffer);
    assert ( VK_SUCCESS == r );

    r_PresentFrame();

    sleep(1);

    vkDeviceWaitIdle(device);

    d_CleanUp();
    r_CleanUp();
    return 0;
}
