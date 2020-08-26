#include "def.h"
#include "r_render.h"
#include "v_video.h"
#include "r_pipeline.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


VkRenderPass swapchainRenderPass;
VkRenderPass offscreenRenderPass;

FrameBuffer  offscreenFrameBuffer;

Frame        frames[FRAME_COUNT];
uint32_t     curFrameIndex = 0;

static void initFrames(void)
{
    VkResult r;
    const VkCommandPoolCreateInfo cmdPoolCi = {
        .queueFamilyIndex = graphicsQueueFamilyIndex,
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    };

    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        r = vkCreateCommandPool(device, &cmdPoolCi, NULL, &frames[i].commandPool);
        assert(r == VK_SUCCESS);

        const VkCommandBufferAllocateInfo allocInfo = {
            .commandBufferCount = 1,
            .commandPool = frames[i].commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
        };

        r = vkAllocateCommandBuffers(device, &allocInfo, &frames[i].commandBuffer);
        // spec states that the last parm is an array of commandBuffers... hoping a pointer
        // to a single one works just as well
        assert( VK_SUCCESS == r );

        const VkSemaphoreCreateInfo semaCi = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        r = vkCreateSemaphore(device, &semaCi, NULL, &frames[i].semaphore);
        assert( VK_SUCCESS == r );

        const VkFenceCreateInfo fenceCi = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        r = vkCreateFence(device, &fenceCi, NULL, &frames[i].fence);
        assert( VK_SUCCESS == r );

        frames[i].index = i;
        frames[i].pImage = &swapchainImages[i];

        VkImageSubresourceRange ssr = {
            .baseArrayLayer = 0,
            .layerCount = 1,
            .baseMipLevel = 0,
            .levelCount = 1,
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
        };

        VkImageViewCreateInfo imageViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .subresourceRange = ssr,
            .format = swapFormat,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .image = *frames[i].pImage,
        };

        r = vkCreateImageView(device, &imageViewInfo, NULL, &frames[i].imageView);
        assert( VK_SUCCESS == r );
    }
    V1_PRINT("Frames successfully initialized.\n");
}

static void initRenderPasses(void)
{
    VkAttachmentDescription attachment = {
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .format = swapFormat,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .samples = VK_SAMPLE_COUNT_1_BIT, // TODO look into what this means
    };

    const VkAttachmentReference reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    const VkSubpassDescription subpass = {
        .inputAttachmentCount = 0,
        .preserveAttachmentCount = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &reference,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    };

    const VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo ci = {
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .dependencyCount = 1,
        .pDependencies = &dependency,
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
    };

    VkResult r = vkCreateRenderPass(device, &ci, NULL, &swapchainRenderPass);
    assert( VK_SUCCESS == r );

    attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    r = vkCreateRenderPass(device, &ci, NULL, &offscreenRenderPass);
    assert( VK_SUCCESS == r );
}

static void initOffscreenFrameBuffer(void)
{
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent = {WINDOW_WIDTH, WINDOW_HEIGHT, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &graphicsQueueFamilyIndex,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkResult r;

    r = vkCreateImage(device, &imageInfo, NULL, &offscreenFrameBuffer.image.handle);
    assert( VK_SUCCESS == r );

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, offscreenFrameBuffer.image.handle, &memReqs);

    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = offscreenFrameBuffer.image.handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .components = {0, 0, 0, 0}, // no swizzling
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    r = vkCreateImageView(device, &viewInfo, NULL, &offscreenFrameBuffer.image.view);
    assert( VK_SUCCESS == r );
}

static void initFrameBuffers(void)
{
    VkResult r;
    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        const VkFramebufferCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .layers = 1,
            .renderPass = swapchainRenderPass,
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .attachmentCount = 1,
            .pAttachments = &frames[i].imageView,
        };

        frames[i].pRenderPass = &swapchainRenderPass;

        r = vkCreateFramebuffer(device, &ci, NULL, &frames[i].frameBuffer);
        assert( VK_SUCCESS == r );
    }

    initOffscreenFrameBuffer();
}

void r_Init(void)
{
    initRenderPasses();
    initFrames();
    initFrameBuffers();
    initDescriptorSets();
    initPipelines();
}

void r_WaitOnQueueSubmit(void)
{
    vkWaitForFences(device, 1, &frames[curFrameIndex].fence, VK_TRUE, UINT64_MAX);
}

Frame* r_RequestFrame(void)
{
    VkResult r;
    uint32_t i = frameCounter % FRAME_COUNT;
    r = vkAcquireNextImageKHR(device, 
            swapchain, 
            UINT64_MAX, 
            imageAcquiredSemaphores[i], 
            VK_NULL_HANDLE, 
            &curFrameIndex);
    assert(VK_SUCCESS == r);
    frameCounter++;
    return &frames[curFrameIndex];
}

void r_PresentFrame(void)
{
    VkResult res;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitDstStageMask = &stageFlags,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAcquiredSemaphores[curFrameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frames[curFrameIndex].semaphore,
        .commandBufferCount = 1,
        .pCommandBuffers = &frames[curFrameIndex].commandBuffer,
    };

    vkWaitForFences(device, 1, &frames[curFrameIndex].fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frames[curFrameIndex].fence);

    res = vkQueueSubmit(graphicsQueues[0], 1, &si, frames[curFrameIndex].fence);
    assert( VK_SUCCESS == res );

    VkResult r;
    const VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frames[curFrameIndex].semaphore,
        .pResults = &r,
        .pImageIndices = &curFrameIndex,
    };

    res = vkQueuePresentKHR(presentQueue, &info);
    assert( VK_SUCCESS == r );
    assert( VK_SUCCESS == res );
}

void r_CleanUp(void)
{
    cleanUpPipelines();
    vkDestroyRenderPass(device, swapchainRenderPass, NULL);
    vkDestroyRenderPass(device, offscreenRenderPass, NULL);
    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        vkDestroyFence(device, frames[i].fence, NULL);
        vkDestroyImageView(device, frames[i].imageView, NULL);
        vkDestroyFramebuffer(device, frames[i].frameBuffer, NULL);
        vkDestroySemaphore(device, frames[i].semaphore, NULL);
        vkDestroyCommandPool(device, frames[i].commandPool, NULL);
    }
}

