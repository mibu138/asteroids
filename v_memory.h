#ifndef V_MEMORY_H
#define V_MEMORY_H

#include "v_def.h"
#include <stdbool.h>

typedef struct {
    size_t    size;
    uint8_t*  address;
    int       vOffset;
    VkBuffer* vBuffer;
    bool      isMapped;
} V_block;

extern uint8_t* hostBuffer;

void v_InitMemory(void);

V_block* v_RequestBlock(size_t size);

void v_CleanUpMemory(void);

#endif /* end of include guard: V_MEMORY_H */
