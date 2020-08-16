#ifndef I_INPUT_H
#define I_INPUT_H

#include <stdint.h>

typedef enum {
    keydown,
    keyup,
} I_EventType;

typedef struct i_event {
    I_EventType type;
    uint32_t    data;
} I_Event;

void i_Init(void);
void i_GetEvents(void);

#endif /* end of include guard: I_INPUT_H */
