/*
r_render.c
 */
#ifndef R_INIT_H
#define R_INIT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.h>

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

void   r_Init(void);
Frame* r_RequestFrame(void);
void   r_PresentFrame(void);
void   r_Destroy(void);

#endif /* end of include guard: R_INIT_H */
