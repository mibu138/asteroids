#include "r_init.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

static VkInstance instance;
static VkPhysicalDevice physicalDevice;
static VkDevice device;

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
}
static void enumeratePhysicalDevices(VkInstance* instance)
{
}

void r_InitVkInstance(void)
{
    uint32_t vulkver = getVkVersion();

    const char appName[] =    "Alison"; 
    const char engineName[] =  "Sword";

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .applicationVersion = 1.0,
        .pApplicationName = appName,
        .pEngineName = engineName,
        .apiVersion = vulkver,
    };

    // one for best practices
    // second one is interesting, sounds like it allows
    // printf to be called from shaders.
    const VkValidationFeatureEnableEXT valfeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
    };

    const VkValidationFeaturesEXT extraValidation = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .disabledValidationFeatureCount = 0,
        .enabledValidationFeatureCount = sizeof(valfeatures) / sizeof(VkValidationFeatureEnableEXT),
        .pEnabledValidationFeatures = valfeatures
    };

    const char* enabledLayers[] =     {"VK_LAYER_KHRONOS_validation"};
    const char* enabledExtensions[] =          {"VK_KHR_xcb_surface"};

    const VkInstanceCreateInfo instanceInfo = {
        .enabledLayerCount = sizeof(enabledExtensions) / sizeof(char*),
        .enabledExtensionCount = sizeof(enabledLayers) / sizeof(char*),
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

void r_InitPhysicalDevice(void)
{
    uint32_t physdevcount;
    VkResult r = vkEnumeratePhysicalDevices(instance, &physdevcount, NULL);
    assert(r == VK_SUCCESS);
    VkPhysicalDevice devices[physdevcount];
    r = vkEnumeratePhysicalDevices(instance, &physdevcount, devices);
    assert(r == VK_SUCCESS);
    printf("Physical device count: %d\n", physdevcount);
    printf("Physical device names:\n");
    for (int i = 0; i < physdevcount; i++) 
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);
        printf("%s\n", props.deviceName);
    }
    printf("%s\n", "Selecting device 0");
    physicalDevice = devices[0];
}

void r_InitDevice(void)
{
    const VkDeviceCreateInfo info = {
        .
    }
}

void r_Init(void)
{
    r_InitVkInstance();
    r_InitPhysicalDevice();
    r_InitDevice();
}

void r_CleanUp(void)
{
    vkDestroyInstance(instance, NULL);
}

