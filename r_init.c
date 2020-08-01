#include "r_init.h"
#include "d_init.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xcb.h>


static VkInstance       instance;
static VkDebugUtilsMessengerEXT debugMessenger;
static VkPhysicalDevice physicalDevice;
static VkSurfaceKHR     surface;
static VkSwapchainKHR   swapchain;

VkDevice device;

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
{
    printf("%s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

static uint32_t getVkVersion(void)
{
    uint32_t v;
    vkEnumerateInstanceVersion(&v);
    uint32_t major = VK_VERSION_MAJOR(v);
    uint32_t minor = VK_VERSION_MINOR(v);
    uint32_t patch = VK_VERSION_PATCH(v);
    printf("Vulkan Version: %d.%d.%d\n", major, minor, patch);
    return v;
}

static void inspectAvailableLayers(void)
{
    uint32_t availableCount;
    vkEnumerateInstanceLayerProperties(&availableCount, NULL);
    VkLayerProperties propertiesAvailable[availableCount];
    vkEnumerateInstanceLayerProperties(&availableCount, propertiesAvailable);
    printf("%s\n", "Vulkan Instance layers available:");
    const int padding = 90;
    for (int i = 0; i < padding; i++) {
        putchar('-');   
    }
    putchar('\n');
    for (int i = 0; i < availableCount; i++) {
        const char* name = propertiesAvailable[i].layerName;
        const char* desc = propertiesAvailable[i].description;
        const int pad = padding - strlen(name);
        printf("%s%*s\n", name, pad, desc );
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
    printf("%s\n", "Vulkan Instance extensions available:");
    for (int i = 0; i < availableCount; i++) {
        printf("%s\n", propertiesAvailable[i].extensionName);
    }
    putchar('\n');
}

static void initVkInstance(void)
{
    uint32_t vulkver = getVkVersion();

    const char appName[] =    "Asteroids"; 
    const char engineName[] =  "Sword";

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .applicationVersion = 1.0,
        .pApplicationName = appName,
        .pEngineName = engineName,
        .apiVersion = vulkver,
    };

#ifdef VERBOSE
    inspectAvailableLayers();
    inspectAvailableExtensions();
#endif

    // one for best practices
    // second one is interesting, sounds like it allows
    // printf to be called from shaders.
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
    printf("Successfully initialized Vulkan instance.\n");
}

static void initDebugMessenger(void)
{
    const VkDebugUtilsMessengerCreateInfoEXT ci = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
//                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
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
    printf("Physical device count: %d\n", physdevcount);
    printf("Physical device names:\n");
    for (int i = 0; i < physdevcount; i++) 
    {
        vkGetPhysicalDeviceProperties(devices[i], &props[i]);
        printf("%s\n", props[i].deviceName);
    }
    printf("Selecting Device: %s\n", props[0].deviceName);
    return devices[0];
}

static void initDevice(void)
{
    physicalDevice = retrievePhysicalDevice();

    uint32_t qfcount;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfcount, NULL);

    VkQueueFamilyProperties qfprops[qfcount];

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfcount, qfprops);

    for (int i = 0; i < qfcount; i++) 
    {
        VkQueryControlFlags flags = qfprops[i].queueFlags;
        printf("Queue Family %d: count: %d flags: ", i, qfprops[i].queueCount);
        if (flags & VK_QUEUE_GRAPHICS_BIT)  printf(" Graphics ");
        if (flags & VK_QUEUE_COMPUTE_BIT)   printf(" Compute ");
        if (flags & VK_QUEUE_TRANSFER_BIT)  printf(" Tranfer ");
        printf("\n");
    }

    const float priorities[] = {1.0};

    const VkDeviceQueueCreateInfo qci[] = { 
        { 
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = 0,
            .queueCount = 1,
            .pQueuePriorities = priorities
        }
    };

    const char* extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const VkDeviceCreateInfo dci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = qci,
        .queueCreateInfoCount = sizeof(qci) / sizeof(qci[0]),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = extensions
    };

    VkResult r = vkCreateDevice(physicalDevice, &dci, NULL, &device);
    assert(r == VK_SUCCESS);
    printf("Device created successfully.\n");
}

static void initSurface(void)
{
    const VkXcbSurfaceCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = xcbWindow.connection,
        .window = xcbWindow.window,
    };

    VkResult r = vkCreateXcbSurfaceKHR(instance, &ci, NULL, &surface);
    assert(r == VK_SUCCESS);
    printf("Surface created successfully.\n");
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

    printf("Surface formats: \n");
    for (int i = 0; i < formatsCount; i++) {
        printf("Format: %d   Colorspace: %d\n", surfaceFormats[i].format, surfaceFormats[i].colorSpace);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

    const VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // i already know its supported 

    assert(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    printf("Surface Capabilities: Min swapchain image count: %d\n", capabilities.minImageCount);

    const VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = 2,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB, //50
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

    printf("Called initSwapchain\n");

    r = vkCreateSwapchainKHR(device, &ci, NULL, &swapchain);
    assert(r == VK_SUCCESS);
    printf("Swapchain created successfully.\n");
}

void r_Init(void)
{
    initVkInstance();
    initDebugMessenger();
    initDevice();
    initSurface();
    initSwapchain();
}

void r_CleanUp(void)
{
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
}

