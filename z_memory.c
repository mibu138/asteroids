#include "z_memory.h"
#include "def.h"
#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include <assert.h>

#define MEMORY_SIZE 262144 // minimum size for an allocation on my device (a validation message told me)
#define BUFFER_SIZE 2048

static VkDeviceMemory hostVisibleCoherentMemory;
static VkBuffer hostMappedBuffer;
static VkPhysicalDeviceMemoryProperties memoryProperties;
static int hostVisibleCoherentIndex;

static void printBufferMemoryReqs(const VkMemoryRequirements* reqs)
{
    printf("Size: %ld\tAlignment: %ld\n", reqs->size, reqs->alignment);
}

void z_Init(void)
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    V1_PRINT("Memory Heap Info:\n");
    for (int i = 0; i < memoryProperties.memoryHeapCount; i++) 
    {
        V1_PRINT("Heap %d: Size %ld: %s local\n", 
                i,
                memoryProperties.memoryHeaps[i].size, 
                memoryProperties.memoryHeaps[i].flags 
                & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? "Device" : "Host");
                // note there are other possible flags, but seem to only deal with multiple gpus
    }

    bool found = false;
    V1_PRINT("Memory Type Info:\n");
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++) 
    {
        VkMemoryPropertyFlags flags = memoryProperties.memoryTypes[i].propertyFlags;
        V1_PRINT("Type %d: Heap Index: %d Flags: | %s%s%s\n", 
                i, 
                memoryProperties.memoryTypes[i].heapIndex,
                flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ?  "Device Local | " : "",
                flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ?  "Host Visible | " : "",
                flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ? "Host Coherent | " : ""
                );   
        if (flags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            hostVisibleCoherentIndex = i;
            found = true;
        }
    }

    assert( found );

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MEMORY_SIZE,
        .memoryTypeIndex = hostVisibleCoherentIndex
    };

    vkAllocateMemory(device, &allocInfo, NULL, &hostVisibleCoherentMemory);

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // queue determined by first use
        .size = BUFFER_SIZE
    };

    vkCreateBuffer(device, &ci, NULL, &hostMappedBuffer);

    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device, hostMappedBuffer, &reqs);
    // we dont need to check the reqs. spec states that 
    // any buffer created without SPARSITY flags will 
    // support being bound to host visible | host coherent

    vkBindBufferMemory(device, hostMappedBuffer, hostVisibleCoherentMemory, 0);
}

int z_Alloc(size_t size, bool mappable, Z_block* block)
{
    return Z_FAILURE;
}

void z_CleanUp()
{
    vkDestroyBuffer(device, hostMappedBuffer, NULL);
    vkFreeMemory(device, hostVisibleCoherentMemory, NULL);
};
