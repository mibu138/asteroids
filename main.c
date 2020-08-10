#include "d_display.h"
#include "r_render.h"
#include "g_game.h"
#include "z_memory.h"
#include "utils.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

int main(int argc, char *argv[])
{
    d_Init();
    printf("Display initialized.\n");
    r_Init();
    printf("Renderer initialized.\n");
    z_Init();
    printf("Memory initialized.\n");
    w_Init();
    printf("World initialized.\n");
    g_Init();
    printf("Game initialized.\n");

    printWorld(&world);

    sleep(1);

    vkDeviceWaitIdle(device);

    g_Destroy();
    w_Destroy();
    d_Destroy();
    z_Destroy();
    r_Destroy();
    return 0;
}
