#include "r_init.h"
#include <vulkan/vulkan_core.h>
#include <stdio.h>
#include <assert.h>


int main(int argc, char *argv[])
{
    r_Init();

    r_CleanUp();

    return 0;
}
