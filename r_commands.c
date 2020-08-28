#include "r_commands.h"
#include "r_render.h"
#include "v_video.h"
#include "w_world.h"
#include "r_pipeline.h"
#include "def.h"
#include <assert.h>
#include <vulkan/vulkan_core.h>

void r_InitRenderCommands(void)
{
    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        r_RequestFrame();
        
        r_UpdateRenderCommands();

        r_PresentFrame();
    }
}

void r_UpdateRenderCommands(void)
{
    VkResult r;
    Frame* frame = &frames[curFrameIndex];
    vkResetCommandPool(device, frame->commandPool, 0);
    VkCommandBufferBeginInfo cbbi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    r = vkBeginCommandBuffer(frame->commandBuffer, &cbbi);

    VkClearValue clearValue = {0.005f, 0.0f, 0.01f, 1.0f};

    const VkRenderPassBeginInfo rpassInfoFirst = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .clearValueCount = 1,
        .pClearValues = &clearValue,
        .renderArea = {{0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}},
        .renderPass = *offscreenFrameBuffer.pRenderPass,
        .framebuffer = offscreenFrameBuffer.handle
    };

    vkCmdBeginRenderPass(frame->commandBuffer, &rpassInfoFirst, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[R_OBJ_PIPELINE]);

    VkDeviceSize vertBufferOffset = w_ObjectVertexBlock->vOffset; 

    assert( 0 == vertBufferOffset ); // should be the offset of the very first vert in the buffer
                                    // which happens to be 0 right now (though does not have to be)
    vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, w_ObjectVertexBlock->vBuffer, &vertBufferOffset);

    for (int i = 0; i < w_ObjectCount; i++) 
    {
        vkCmdDraw(frame->commandBuffer, w_Geos[i].vertCount, 1, w_Geos[i].vertIndex, 0);
    }

    vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[R_EMIT_PIPELINE]);

    VkDeviceSize emitVertBufferOffset = w_EmitableVertexBlock->vOffset;

    vkCmdBindVertexBuffers(frame->commandBuffer, 0, 1, w_EmitableVertexBlock->vBuffer, &emitVertBufferOffset);

    if (w_EmitableCount)
        vkCmdDraw(frame->commandBuffer, w_EmitableCount, 1, 0, 0);

    vkCmdEndRenderPass(frame->commandBuffer);

    VkRenderPassBeginInfo rpassInfoSecond = rpassInfoFirst;
    rpassInfoSecond.framebuffer =  frame->frameBuffer;
    rpassInfoSecond.renderPass  = *frame->pRenderPass;

    vkCmdBeginRenderPass(frame->commandBuffer, &rpassInfoSecond, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(frame->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[R_POST_PROC_PIPELINE]);

    vkCmdBindDescriptorSets(frame->commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutPostProcess, 
            0, 1, descriptorSets, 0, NULL);

    vkCmdDraw(frame->commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(frame->commandBuffer);

    r = vkEndCommandBuffer(frame->commandBuffer);
    assert ( VK_SUCCESS == r );
}
