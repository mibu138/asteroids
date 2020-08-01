#include "d_init.h"
#include "r_init.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    d_Init();
    r_Init();

    sleep(2);

    d_CleanUp();
    r_CleanUp();
    return 0;
}
