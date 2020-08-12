#ifndef Z_MEMORY_H
#define Z_MEMORY_H

#include "r_render.h"
#include <stdbool.h>

#define Z_SUCCESS 1
#define Z_FAILURE 0

typedef struct z_block {
    size_t    size;
    int       offset;
    void*     address;
    VkBuffer* vBuffer;
    bool      isMapped;
} Z_block;

void z_Init(void);

// returns Z_SUCCESS on success, otherwise an error occurred
int z_Alloc(size_t size, bool mappable, Z_block*);

void z_CleanUp(void);

#endif /* end of include guard: Z_MEMORY_H */
