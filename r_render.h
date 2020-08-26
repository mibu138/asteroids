/*
r_render.c
 */
#ifndef R_INIT_H
#define R_INIT_H

#define VK_USE_PLATFORM_XCB_KHR
#define MAX_PIPELINES 4
#define MAX_DESCRIPTOR_SETS 2
#define FRAME_COUNT 2

#include <vulkan/vulkan.h>

typedef struct {
    VkImage         handle;
    VkImageView     view;
    VkSampler       sampler;
} Image;

typedef struct {
    VkFramebuffer   handle;
    Image           image;
    VkRenderPass*   pRenderPass;
} FrameBuffer;

typedef struct frame {
    VkCommandPool   commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore     semaphore;
    VkFence         fence;
    VkImage*        pImage;
    VkImageView     imageView;
    VkFramebuffer   frameBuffer;
    VkRenderPass*   pRenderPass;
    uint32_t        index;
} Frame;

extern VkDevice device;
extern VkPhysicalDevice physicalDevice;
extern VkPipeline pipelines[MAX_PIPELINES];
extern VkRenderPass swapchainRenderPass;
extern Frame frames[FRAME_COUNT];
extern uint32_t curFrameIndex;

void   r_Init(void);
void   r_WaitOnQueueSubmit(void);
Frame* r_RequestFrame(void);
void   r_PresentFrame(void);
void   r_CleanUp(void);

#endif /* end of include guard: R_INIT_H */
