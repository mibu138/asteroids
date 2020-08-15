#include "d_display.h"
#include "m_math.h"
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
    printf("Buffer print:\n");
    printBuffer(hostBuffer, sizeof(Vec2) * 24, Vec2, printVec2);

    int i = 0;
    while( i++ < 10 )
    {
        r_RequestFrame();
        r_PresentFrame();
        printf("Presenting frame\n");
        sleep(1);
    }

    vkDeviceWaitIdle(device);

    g_CleanUp();
    w_CleanUp();
    d_CleanUp();
    z_CleanUp();
    r_CleanUp();
    return 0;
}
