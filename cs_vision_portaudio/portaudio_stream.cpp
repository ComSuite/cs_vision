#include "portaudio_stream.h"

void print_device_list()
{
    for (int i = 0; i < Pa_GetDeviceCount(); i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        printf("[%d] %s\n", i, info->name);
    }
}

int get_device_id(const char* name)
{
    for (int i = 0; i < Pa_GetDeviceCount(); i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info != nullptr) {
            const char* p = strstr(info->name, name);
            if (p == info->name) {
                return i;
            }
        }
    }

    return -1;
}

int init_audio_stream(const char* input_device_name, const char* output_device_name, PaStream** stream, int& num_channels, PaStreamCallback* callback, void* user_data)
{
    PaStreamParameters input_parameters;
    PaStreamParameters output_parameters;
    PaStreamParameters* p_input_parameters = NULL;
    PaStreamParameters* p_output_parameters = NULL;
    const PaDeviceInfo* input_info = nullptr;
    const PaDeviceInfo* output_info = nullptr;
    int max_input_channels = 0;
    int max_output_channels = 0;

    printf("sizeof(int) = %lu\n", (unsigned long)sizeof(int));
    printf("sizeof(long) = %lu\n", (unsigned long)sizeof(long));

    if (input_device_name != nullptr) {
        input_parameters.device = get_device_id(input_device_name);
        if (input_parameters.device < 0) {
            printf("Can not find input defice: %s\n", input_device_name);
            return 1;
        }

        printf("Input device # %d.\n", input_parameters.device);
        input_info = Pa_GetDeviceInfo(input_parameters.device);
        printf("        Name: %s\n", input_info->name);
        printf("          LL: %g s\n", input_info->defaultLowInputLatency);
        printf("          HL: %g s\n", input_info->defaultHighInputLatency);
        printf("Max channels: %d\n", input_info->maxInputChannels);

        max_input_channels = input_info->maxInputChannels;
        input_parameters.channelCount = input_info->maxInputChannels;
        input_parameters.sampleFormat = PA_SAMPLE_TYPE;
        input_parameters.suggestedLatency = input_info->defaultHighInputLatency;
        input_parameters.hostApiSpecificStreamInfo = nullptr;

        p_input_parameters = &input_parameters;
    }

    if (output_device_name != nullptr) {
        output_parameters.device = get_device_id(output_device_name);
        if (output_parameters.device < 0) {
            printf("Can not find output defice: %s\n", output_device_name);
            return 2;
        }

        printf("Output device # %d.\n", output_parameters.device);
        output_info = Pa_GetDeviceInfo(output_parameters.device);
        printf("        Name: %s\n", output_info->name);
        printf("          LL: %g s\n", output_info->defaultLowOutputLatency);
        printf("          HL: %g s\n", output_info->defaultHighOutputLatency);
        printf("Max channels: %d\n", output_info->maxOutputChannels);

        max_output_channels = output_info->maxOutputChannels;
        output_parameters.channelCount = output_info->maxOutputChannels;
        output_parameters.sampleFormat = PA_SAMPLE_TYPE;
        output_parameters.suggestedLatency = output_info->defaultHighOutputLatency;
        output_parameters.hostApiSpecificStreamInfo = nullptr;

        p_output_parameters = &output_parameters;
    }

    if (max_input_channels > 0 && max_output_channels > 0) {
        num_channels = max_input_channels < max_output_channels ? max_input_channels : max_output_channels;
        printf("Num channels = %d.\n", num_channels);

        input_parameters.channelCount = num_channels;
        output_parameters.channelCount = num_channels;
    }

    PaError err = Pa_OpenStream(
        stream,
        p_input_parameters,
        p_output_parameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        callback,
        user_data);

    return err;
}

