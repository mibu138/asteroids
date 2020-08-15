#include "g_game.h"
#include "r_render.h"
#include "d_display.h"
#include "def.h"
#include <assert.h>
#include <vulkan/vulkan_core.h>

Player player;

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

        VkDeviceSize vertBufferOffset = w_VertexBlock->vOffset; // first element gives us the offset

        assert( 0 == vertBufferOffset ); // should be the offset of the very first vert in the buffer
                                        // which happens to be 0 right now (though does not have to be)

        vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);

        vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, w_VertexBlock->vBuffer, &vertBufferOffset);

        for (int i = 0; i < w_ObjectCount; i++) 
        {
            vkCmdDraw(frame->commandBuffer, w_Geos[i].vertCount, 1, w_Geos[i].vertIndex, 0);
        }

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

void g_CleanUp(void)
{
}

