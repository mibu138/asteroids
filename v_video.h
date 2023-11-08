/*
v_video.c
 */
#ifndef V_VIDEO_H
#define V_VIDEO_H

#include "v_def.h"

extern VkDevice         device;
extern VkPhysicalDevice physicalDevice;
extern uint32_t graphicsQueueFamilyIndex;
extern VkQueue  graphicsQueues[G_QUEUE_COUNT];
extern VkQueue  presentQueue;

extern VkImage        swapchainImages[FRAME_COUNT];
extern const VkFormat swapFormat;
extern VkSwapchainKHR   swapchain;
extern VkSemaphore    imageAcquiredSemaphores[FRAME_COUNT];
extern uint64_t       frameCounter;

void v_Init(void);
void v_RecreateSwapChain();
void v_AcquireSwapImage(uint32_t* pImageIndex);
void v_CleanUp(void);

#endif /* end of include guard: V_VIDEO_H */
