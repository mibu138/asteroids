#ifndef R_INIT_H
#define R_INIT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.h>

extern VkDevice device;

void r_Init(void);
void r_CleanUp(void);

#endif /* end of include guard: R_INIT_H */
