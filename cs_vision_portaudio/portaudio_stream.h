#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _OPEN_THREADS
#include <pthread.h>
#include <portaudio.h>

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#ifdef _WIN32
#define SAMPLE_RATE       (44100)
#else
#define SAMPLE_RATE       (48000)
#endif
#define FRAMES_PER_BUFFER   (512)
#define NUM_SECONDS          (0)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG           (0)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 0
#define PA_SAMPLE_TYPE  paInt16
#define SAMPLE_SIZE (2)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt24
#define SAMPLE_SIZE (3)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

void print_device_list();
int init_audio_stream(const char* input_device_name, const char* output_device_name, PaStream** stream, 
	int& num_channels, PaStreamCallback* callback, void* user_data);
