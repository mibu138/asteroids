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
    printf("Acquired frame. Image: 0x%016lX Index: %d\n", (unsigned long)*frame->pImage, frame->index);

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
    printf("Presented\n");

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
    printf("Presented\n");

    sleep(1);

    printf("Lets go to epilepsy mode now.\n");
    sleep(1);
    printf("3...\n");
    sleep(1);
    printf("2...\n");
    sleep(1);
    printf("1...\n");
    sleep(1);

    int dur = 1000000;
    for (int i = 0; i < 1000; i++) 
    {
        frame = r_RequestFrame();
        r_PresentFrame();
        usleep(dur);
        dur /= 1.1;
        if (i == 10) printf("Warming up.\n");
        if (i == 100) printf("Gettin steamy.\n");
        if (i == 500) printf("Are you feelin it now, Mr. Krabs?\n");
    }
     
    vkDeviceWaitIdle(device);

    d_CleanUp();
    r_CleanUp();
    return 0;
}
