#include "d_display.h"
#include "m_math.h"
#include "r_render.h"
#include "g_game.h"
#include "z_memory.h"
#include "i_input.h"
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
    i_Init();
    printf("Input initialized\n");

    printWorld();

    while( 1 ) 
    {
        i_GetEvents();
        i_ProcessEvents();
        r_WaitOnQueueSubmit();
        w_Update();
        r_RequestFrame();
        r_PresentFrame();
        usleep(30000);
    }

    vkDeviceWaitIdle(device);

    i_CleanUp();
    g_CleanUp();
    w_CleanUp();
    d_CleanUp();
    z_CleanUp();
    r_CleanUp();
    return 0;
}
