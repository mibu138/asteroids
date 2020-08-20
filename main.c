#include "d_display.h"
#include "m_math.h"
#include "r_render.h"
#include "g_game.h"
#include "z_memory.h"
#include "i_input.h"
#include "utils.h"
#include "def.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>
#include <time.h>

jmp_buf exit_game;

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

    struct timespec startTime;
    struct timespec endTime;

    unsigned long frameCount = 0;
    unsigned long nsElapsed  = 0;

    while( 1 ) 
    {
        if (setjmp(exit_game)) break;

        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);
        
        i_GetEvents();
        i_ProcessEvents();
        g_Update();
        w_Update();
        r_RequestFrame();
        r_PresentFrame();

        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endTime);

        nsElapsed += endTime.tv_nsec - startTime.tv_nsec;

        usleep(30000);
        frameCount++;
    }

    printf("Total Frames: %ld\n", frameCount);
    printf("Total nanoseconds: %ld\n", nsElapsed);
    //printf("Total seconds: %ld\n", sElapsed);
    printf("Average nanoseconds per frame: %ld\n", nsElapsed / frameCount);

    vkDeviceWaitIdle(device);

    i_CleanUp();
    g_CleanUp();
    w_CleanUp();
    d_CleanUp();
    z_CleanUp();
    r_CleanUp();
    return 0;
}
