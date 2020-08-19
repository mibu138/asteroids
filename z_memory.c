#include "z_memory.h"
#include "def.h"
#include <stdio.h>
#include <vulkan/vulkan_core.h>
#include <assert.h>

#define MEMORY_SIZE 262144 // minimum size for an allocation on my device (a validation message told me)
#define BUFFER_SIZE 262144
#define MAX_BLOCKS 256

static VkDeviceMemory hostVisibleCoherentMemory;
static VkBuffer hostMappedBuffer;
static VkPhysicalDeviceMemoryProperties memoryProperties;
static int hostVisibleCoherentIndex;
uint8_t* hostBuffer;
static Z_block blocks[MAX_BLOCKS];

static int blockCount = 0;
static int bytesAvailable = BUFFER_SIZE;
static int curBufferOffset = 0;

static void printBufferMemoryReqs(const VkMemoryRequirements* reqs)
{
    printf("Size: %ld\tAlignment: %ld\n", reqs->size, reqs->alignment);
}

void z_Init(void)
{
    VkResult r;

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

    r = vkAllocateMemory(device, &allocInfo, NULL, &hostVisibleCoherentMemory);
    assert( VK_SUCCESS == r );

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // queue determined by first use
        .size = BUFFER_SIZE
    };


    r = vkCreateBuffer(device, &ci, NULL, &hostMappedBuffer);
    assert( VK_SUCCESS == r );

    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device, hostMappedBuffer, &reqs);
    // we dont need to check the reqs. spec states that 
    // any buffer created without SPARSITY flags will 
    // support being bound to host visible | host coherent

    r = vkBindBufferMemory(device, hostMappedBuffer, hostVisibleCoherentMemory, 0);
    assert( VK_SUCCESS == r );

    r = vkMapMemory(device, hostVisibleCoherentMemory, 0, BUFFER_SIZE, 0, (void**)&hostBuffer);
    assert( VK_SUCCESS == r );
}

Z_block* z_RequestBlock(const size_t size)
{
    assert( size % 4 == 0 ); // only allow for word-sized blocks
    assert( size < bytesAvailable);
    assert( blockCount < MAX_BLOCKS );
    Z_block* pBlock = &blocks[blockCount];
    pBlock->address = hostBuffer + curBufferOffset;
    pBlock->vBuffer = &hostMappedBuffer;
    pBlock->size = size;
    pBlock->vOffset = curBufferOffset;
    pBlock->isMapped = true;

    bytesAvailable -= size;
    curBufferOffset += size;
    blockCount++;
    // we really do need to be worrying about alignment here.
    // anything that is not a multiple of 4 bytes will have issues.
    // there is VERY GOOD CHANCE that there are other alignment
    // issues to consider.
    //
    // we should probably divide up the buffer into Chunks, where
    // all the blocks in a chunk contain the same kind of element
    // (a chunk for vertices, a chunk for indices, a chunk for 
    // uniform matrices, etc).

    return pBlock;
}

void z_CleanUp()
{
    vkUnmapMemory(device, hostVisibleCoherentMemory);
    vkDestroyBuffer(device, hostMappedBuffer, NULL);
    vkFreeMemory(device, hostVisibleCoherentMemory, NULL);
};
