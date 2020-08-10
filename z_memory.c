#include "z_memory.h"
#include <vulkan/vulkan_core.h>

#define BUFFER_SIZE 1024

static VkBuffer hostMappedBuffer;

void z_Init(void)
{
    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // queue determined by first use
        .size = BUFFER_SIZE
    };

    vkCreateBuffer(device, &ci, NULL, &hostMappedBuffer);

    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device, hostMappedBuffer, &reqs);
}

int z_Alloc(size_t size, bool mappable, Z_block* block)
{
    return Z_FAILURE;
}

void z_Destroy()
{
    vkDestroyBuffer(device, hostMappedBuffer, NULL);
}
