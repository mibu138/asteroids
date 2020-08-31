#ifndef A_AUDIO_H
#define A_AUDIO_H

#include <stdint.h>

#define A_SAMPLE_RATE 44100
#define A_BUFFER_SIZE 2048

typedef struct 
{
    float buffer[A_BUFFER_SIZE];
} Wave;

typedef struct
{
    const uint32_t    duration;
    uint32_t          ticks;
    float             amplitude;
    uint32_t          phase;
    float             frequency;
    Wave              wave;
} Sound;

void a_Init(void);
void a_PlayBeep(void);
void a_CleanUp(void);

#endif /* end of include guard: A_AUDIO_H */
