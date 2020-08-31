#include "a_audio.h"
#include <stdbool.h>
#include <math.h>
#include <portaudio.h>
#include <assert.h>

PaStream* a_Stream;
Sound     soundA;

Wave      sineWave;
Wave      squareWave;

static int waveFunction(
        const void*     inputBuffer,
        void* restrict  outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* restrict userData)
{
    Sound* data = (Sound*)userData;
    float* out =   (float*)outputBuffer;
    for (int i = 0; i < framesPerBuffer; i++) 
    {
        bool playing = data->ticks > 0;
        if (playing)
        {
            data->amplitude = 1.0;
            data->ticks--;
        }
        else 
            data->amplitude = 0.0;
        const int spw = A_SAMPLE_RATE / data->frequency;
        const int phaseVel = A_BUFFER_SIZE / spw;
        *out++ = data->amplitude * data->wave.buffer[data->phase];
        data->phase = (data->phase + phaseVel) % A_BUFFER_SIZE;
    }
    return 0;
}

static void initSineWave(void)
{
    double x = 0.0;
    const double step = M_PI * 2 / A_BUFFER_SIZE;
    for (int i = 0; i < A_BUFFER_SIZE; i++) 
    {
        sineWave.buffer[i] = sin(x);
        x += step;
    }
}

void a_Init(void)
{
    PaError r;
    initSineWave();
    soundA.frequency = 320.0;
    soundA.wave = sineWave;

    r = Pa_Initialize();
    assert( paNoError == r );

    r = Pa_OpenDefaultStream(&a_Stream, 
                0, /*input channels*/
                1,  /*output channels*/
                paFloat32, /*sample format*/ 
                A_SAMPLE_RATE, 
                256, /*frames per buffer*/ 
                waveFunction, 
                &soundA);
    assert( paNoError == r );
    r = Pa_StartStream(a_Stream);
    assert( paNoError == r );
}

void a_PlayBeep(void)
{
    soundA.ticks = 4000;
}

void a_CleanUp(void)
{
    PaError r;
    r = Pa_StopStream(a_Stream);
    assert( paNoError == r );
}
