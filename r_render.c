#include "def.h"
#include "r_render.h"
#include "d_display.h"
#include "r_pipeline.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xcb.h>

#define G_QUEUE_COUNT 4

VkDevice device;
VkPhysicalDevice physicalDevice;
VkRenderPass swapchainRenderPass;
VkRenderPass offscreenRenderPass;

FrameBuffer offscreenFrameBuffer;

static VkInstance       instance;
static VkSurfaceKHR     surface;
static VkSwapchainKHR   swapchain;

static VkDebugUtilsMessengerEXT debugMessenger;

static uint32_t graphicsQueueFamilyIndex = UINT32_MAX; //hopefully this causes obvious errors
static VkQueue  graphicsQueues[G_QUEUE_COUNT];
static VkQueue  presentQueue;

Frame         frames[FRAME_COUNT];
uint32_t      curFrameIndex = 0;
static VkSemaphore   imageAcquiredSemaphores[FRAME_COUNT];
static VkImage       swapchainImages[FRAME_COUNT];
static uint64_t      frameCounter = 0;

static const VkFormat swapFormat = VK_FORMAT_B8G8R8A8_SRGB;

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
{
    V1_PRINT("%s\n", pCallbackData->pMessage);
    return VK_FALSE; // application must return false;
}

static uint32_t getVkVersion(void)
{
    uint32_t v;
    vkEnumerateInstanceVersion(&v);
    uint32_t major = VK_VERSION_MAJOR(v);
    uint32_t minor = VK_VERSION_MINOR(v);
    uint32_t patch = VK_VERSION_PATCH(v);
    V1_PRINT("Vulkan Version: %d.%d.%d\n", major, minor, patch);
    return v;
}

static void inspectAvailableLayers(void)
{
    uint32_t availableCount;
    vkEnumerateInstanceLayerProperties(&availableCount, NULL);
    VkLayerProperties propertiesAvailable[availableCount];
    vkEnumerateInstanceLayerProperties(&availableCount, propertiesAvailable);
    V1_PRINT("%s\n", "Vulkan Instance layers available:");
    const int padding = 90;
    for (int i = 0; i < padding; i++) {
        putchar('-');   
    }
    putchar('\n');
    for (int i = 0; i < availableCount; i++) {
        const char* name = propertiesAvailable[i].layerName;
        const char* desc = propertiesAvailable[i].description;
        const int pad = padding - strlen(name);
        V1_PRINT("%s%*s\n", name, pad, desc );
        for (int i = 0; i < padding; i++) {
            putchar('-');   
        }
        putchar('\n');
    }
    putchar('\n');
}

static void inspectAvailableExtensions(void)
{
    uint32_t availableCount;
    vkEnumerateInstanceExtensionProperties(NULL, &availableCount, NULL);
    VkExtensionProperties propertiesAvailable[availableCount];
    vkEnumerateInstanceExtensionProperties(NULL, &availableCount, propertiesAvailable);
    V1_PRINT("%s\n", "Vulkan Instance extensions available:");
    for (int i = 0; i < availableCount; i++) {
        V1_PRINT("%s\n", propertiesAvailable[i].extensionName);
    }
    putchar('\n');
}

static void initVkInstance(void)
{
    uint32_t vulkver = getVkVersion();

    const char appName[] =    "Asteroids"; 
    const char engineName[] = "Sword";

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .applicationVersion = 1.0,
        .pApplicationName = appName,
        .pEngineName = engineName,
        .apiVersion = vulkver,
    };

#if VERBOSE > 1
    inspectAvailableLayers();
    inspectAvailableExtensions();
#endif

    // one for best practices
    // second one is interesting, sounds like it allows
    // V1_PRINT to be called from shaders.
    const VkValidationFeatureEnableEXT valfeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
//        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
//        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
    };

    const VkValidationFeaturesEXT extraValidation = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .disabledValidationFeatureCount = 0,
        .enabledValidationFeatureCount = sizeof(valfeatures) / sizeof(VkValidationFeatureEnableEXT),
        .pEnabledValidationFeatures = valfeatures
    };

    const char* enabledLayers[] =     {
        "VK_LAYER_KHRONOS_validation"
    };
    const char* enabledExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_xcb_surface",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils"
    };

    const VkInstanceCreateInfo instanceInfo = {
        .enabledLayerCount = sizeof(enabledLayers) / sizeof(char*),
        .enabledExtensionCount = sizeof(enabledExtensions) / sizeof(char*),
        .ppEnabledExtensionNames = enabledExtensions,
        .ppEnabledLayerNames = enabledLayers,
        .pApplicationInfo = &appInfo,
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &extraValidation,
    };

    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    assert(result == VK_SUCCESS);
    V1_PRINT("Successfully initialized Vulkan instance.\n");
}

static void initDebugMessenger(void)
{
    const VkDebugUtilsMessengerCreateInfoEXT ci = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
#if VERBOSE > 1
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
#endif
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };

    PFN_vkVoidFunction fn;
    fn = vkGetInstanceProcAddr(
            instance, 
            "vkCreateDebugUtilsMessengerEXT");

    assert(fn);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)fn;

    VkResult r = func(instance, 
            &ci, NULL, &debugMessenger);
    assert(r == VK_SUCCESS);
}

static VkPhysicalDevice retrievePhysicalDevice(void)
{
    uint32_t physdevcount;
    VkResult r = vkEnumeratePhysicalDevices(instance, &physdevcount, NULL);
    assert(r == VK_SUCCESS);
    VkPhysicalDevice devices[physdevcount];
    r = vkEnumeratePhysicalDevices(instance, &physdevcount, devices);
    assert(r == VK_SUCCESS);
    VkPhysicalDeviceProperties props[physdevcount];
    V1_PRINT("Physical device count: %d\n", physdevcount);
    V1_PRINT("Physical device names:\n");
    for (int i = 0; i < physdevcount; i++) 
    {
        vkGetPhysicalDeviceProperties(devices[i], &props[i]);
        V1_PRINT("%s\n", props[i].deviceName);
    }
    V1_PRINT("Selecting Device: %s\n", props[0].deviceName);
    return devices[0];
}

static void initDevice(void)
{
    physicalDevice = retrievePhysicalDevice();
    VkResult r;
    uint32_t qfcount;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfcount, NULL);

    VkQueueFamilyProperties qfprops[qfcount];

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfcount, qfprops);

    for (int i = 0; i < qfcount; i++) 
    {
        VkQueryControlFlags flags = qfprops[i].queueFlags;
        V1_PRINT("Queue Family %d: count: %d flags: ", i, qfprops[i].queueCount);
        if (flags & VK_QUEUE_GRAPHICS_BIT)  V1_PRINT(" Graphics ");
        if (flags & VK_QUEUE_COMPUTE_BIT)   V1_PRINT(" Compute ");
        if (flags & VK_QUEUE_TRANSFER_BIT)  V1_PRINT(" Tranfer ");
        V1_PRINT("\n");
    }

    graphicsQueueFamilyIndex = 0; // because we know this
    assert( G_QUEUE_COUNT < qfprops[graphicsQueueFamilyIndex].queueCount );

    const float priorities[G_QUEUE_COUNT] = {1.0, 1.0, 1.0, 1.0};

    const VkDeviceQueueCreateInfo qci[] = { 
        { 
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphicsQueueFamilyIndex,
            .queueCount = G_QUEUE_COUNT,
            .pQueuePriorities = priorities,
        }
    };

    uint32_t propCount;
    r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &propCount, NULL);
    assert(r == VK_SUCCESS);
    VkExtensionProperties properties[propCount];
    r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &propCount, properties);
    assert(r == VK_SUCCESS);

#if VERBOSE > 1
    V1_PRINT("Device Extensions available: \n");
    for (int i = 0; i < propCount; i++) 
    {
        V1_PRINT("Name: %s    Spec Version: %d\n", properties[i].extensionName, properties[i].specVersion);    
    }
#endif

    const char* extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures availableFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &availableFeatures);

    assert( VK_TRUE == availableFeatures.fillModeNonSolid );

    VkPhysicalDeviceFeatures enabledFeatures = {
        .fillModeNonSolid = VK_TRUE,
        .wideLines = VK_TRUE,
        .largePoints = VK_TRUE
    };

    const VkDeviceCreateInfo dci = {
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .pEnabledFeatures = &enabledFeatures,
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = qci,
        .queueCreateInfoCount = sizeof(qci) / sizeof(qci[0]),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = extensions
    };

    r = vkCreateDevice(physicalDevice, &dci, NULL, &device);
    assert(r == VK_SUCCESS);
    V1_PRINT("Device created successfully.\n");
}

static void initSurface(void)
{
    const VkXcbSurfaceCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = d_XcbWindow.connection,
        .window = d_XcbWindow.window,
    };

    VkResult r = vkCreateXcbSurfaceKHR(instance, &ci, NULL, &surface);
    assert(r == VK_SUCCESS);
    V1_PRINT("Surface created successfully.\n");
}

static void initSwapchain(void)
{
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &supported);

    assert(supported == VK_TRUE);

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult r = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    assert(r == VK_SUCCESS);

    uint32_t formatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, NULL);
    VkSurfaceFormatKHR surfaceFormats[formatsCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, surfaceFormats);

    V1_PRINT("Surface formats: \n");
    for (int i = 0; i < formatsCount; i++) {
        V1_PRINT("Format: %d   Colorspace: %d\n", surfaceFormats[i].format, surfaceFormats[i].colorSpace);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

    const VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // i already know its supported 

    assert(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    V1_PRINT("Surface Capabilities: Min swapchain image count: %d\n", capabilities.minImageCount);

    const VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 2,
        .imageFormat = swapFormat, //50
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = capabilities.currentExtent,
        .imageArrayLayers = 1, // number of views in a multiview / stereo surface
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // queue sharing. see vkspec section 11.7. 
        .queueFamilyIndexCount = 0, // dont need with exclusive sharing
        .pQueueFamilyIndices = NULL, // ditto
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, //dunno. may affect blending
        .presentMode = presentMode,
        .clipped = VK_FALSE, // allows pixels convered by another window to be clipped. but will mess up saving the swap image.
        .oldSwapchain = VK_NULL_HANDLE
    };

    r = vkCreateSwapchainKHR(device, &ci, NULL, &swapchain);
    assert(VK_SUCCESS == r);

    uint32_t imageCount;
    r = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
    assert(VK_SUCCESS == r);
    assert(FRAME_COUNT == imageCount);
    r = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);
    assert(VK_SUCCESS == r);

    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        VkSemaphoreCreateInfo semaCi = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        r = vkCreateSemaphore(device, &semaCi, NULL, &imageAcquiredSemaphores[i]);
    }

    V1_PRINT("Swapchain created successfully.\n");
}

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

static void initQueues(void)
{
    for (int i = 0; i < G_QUEUE_COUNT; i++) 
    {
        vkGetDeviceQueue(device, graphicsQueueFamilyIndex, i, &graphicsQueues[i]);
    }
    presentQueue = graphicsQueues[0]; // use the first queue to present
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
    initVkInstance();
    initDebugMessenger();
    initDevice();
    initSurface();
    initSwapchain();
    initQueues();
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
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        
    cleanUpPipelines();
    vkDestroyRenderPass(device, swapchainRenderPass, NULL);
    vkDestroyRenderPass(device, offscreenRenderPass, NULL);
    for (int i = 0; i < FRAME_COUNT; i++) 
    {
        vkDestroyFence(device, frames[i].fence, NULL);
        vkDestroyImageView(device, frames[i].imageView, NULL);
        vkDestroyFramebuffer(device, frames[i].frameBuffer, NULL);
        vkDestroySemaphore(device, imageAcquiredSemaphores[i], NULL);
        vkDestroySemaphore(device, frames[i].semaphore, NULL);
        vkDestroyCommandPool(device, frames[i].commandPool, NULL);
    }
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    vkDestroyInstance(instance, NULL);
}

