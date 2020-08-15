#ifndef Z_MEMORY_H
#define Z_MEMORY_H

#include "r_render.h"
#include <stdbool.h>

typedef struct z_block {
    size_t    size;
    uint8_t*  address;
    int       vOffset;
    VkBuffer* vBuffer;
    bool      isMapped;
} Z_block;

extern uint8_t* hostBuffer;

void z_Init(void);

// returns Z_SUCCESS on success, otherwise an error occurred
Z_block* z_RequestBlock(size_t size);

void z_CleanUp(void);

#endif /* end of include guard: Z_MEMORY_H */
