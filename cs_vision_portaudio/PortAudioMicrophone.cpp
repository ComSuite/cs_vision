/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2023, Alexander Epstine (a@epstine.com)
 **************************************************************************************
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <cstdio>
#include <iostream>
#include "PortAudioMicrophone.h"
#include "portaudio_stream.h"
#include "audio_types.h"
#include <thread>

using namespace std;
using namespace cv;
using namespace cv::cuda;
using namespace cs;

static int process_sample_callback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    audio_callback_data* data = (audio_callback_data*)userData;
    if (data == nullptr)
        return paContinue;

    PortAudioMicrophone* mic = (PortAudioMicrophone*)data->microphone;
    if (mic != nullptr)
        mic->set_buffer((char*)inputBuffer, framesPerBuffer * SAMPLE_SIZE);

    return paContinue;
}

PortAudioMicrophone::PortAudioMicrophone()
{
}

PortAudioMicrophone::~PortAudioMicrophone()
{
    if (detector_buffer != nullptr)
        free(detector_buffer);
}

int PortAudioMicrophone::info()
{

}

int PortAudioMicrophone::open(std::variant<std::string, int> device)
{
    if (std::holds_alternative<std::string>(device)) {
        return open(std::get<string>(device).c_str());
    }
    else
        return 0;
}

int PortAudioMicrophone::open(const int id) 
{
	return 0;
}

int PortAudioMicrophone::open(const char* name)
{
    if (name == nullptr)
        return 0;

    close();

    audio_data = new audio_callback_data();
    if (audio_data == nullptr)
        return 0;

	PaError err = init_audio_stream(name, nullptr, &stream, channels, process_sample_callback, audio_data);
    if (err != paNoError) {
        std:: cout << "Can not create audio stream. Err=" << err << " (" << Pa_GetErrorText(err) << ") " << std::endl;
        delete audio_data;
        return 0;
    }

    /*
    int num_bytes = FRAMES_PER_BUFFER * channels * SAMPLE_SIZE;
    audio_data->buffer = (char*)malloc(num_bytes);
    if (audio_data->buffer == nullptr) {
        std::cout << "Can not allocate buffer" << std::endl;
        delete audio_data;
        return 0;
    }
    */

    audio_data->start_time = Pa_GetStreamTime(stream);
    audio_data->duration = NUM_SECONDS;
    audio_data->num_channels = channels;
    audio_data->microphone = this;

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cout << "Can`t start portaudio stream" << std::endl;
        //free(sample_block);
        delete audio_data;
        return 0;
    }

    set_ready(false);

    return 1;
}

int PortAudioMicrophone::close()
{
    if (audio_data != nullptr) {
        delete audio_data;
        audio_data = nullptr;
    }

    return 1;
}

int PortAudioMicrophone::prepare()
{
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cout << "PortAudio error: " << err << " (" << Pa_GetErrorText(err) << ")" << std::endl;
        return 0;
    }

    return 1;
}

int PortAudioMicrophone::save_to_file()
{
    return 0;
}

#ifdef  __HAS_CUDA__
int PortAudioMicrophone::get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray)
{
    return 0;
}

int PortAudioMicrophone::get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray)
{
    return 0;
}
#endif 

int PortAudioMicrophone::get_frame(const char* name, cv::Mat& frame, bool convert_to_gray)
{
    return 0;
}

int PortAudioMicrophone::get_frame(cv::Mat& frame, bool convert_to_gray)
{
    if (ready_flag) {
        cv::Mat t(1, detector_input_length, CV_8UC3);
        frame = t;
        memcpy(frame.ptr(), detector_buffer, detector_input_length);

        detector_cur_position = 0;
    }

    return 1;
}

int PortAudioMicrophone::get_width()
{
    return FRAMES_PER_BUFFER;
}

int PortAudioMicrophone::get_height()
{
    return 1;
}

int PortAudioMicrophone::get_fps()
{
    return SAMPLE_RATE;
}

bool PortAudioMicrophone::is_opened()
{
    return true;
}

void PortAudioMicrophone::set_detector_buffer(size_t length)
{
    if (detector_input_length < length) {
        detector_input_length = length;

        if (detector_buffer == nullptr)
            detector_buffer = (char*)malloc(detector_input_length);
        else
            detector_buffer = (char*)realloc(detector_buffer, detector_input_length);
    }
}

void PortAudioMicrophone::set_buffer(char* data, uint length)
{
    if (audio_data == nullptr || data == nullptr || length == 0 || ready_flag)
        return;

    char* p = detector_buffer + detector_cur_position;
    if (size_t free_count = detector_input_length - detector_cur_position; free_count < length) {
        memcpy(p, data, free_count);
        detector_cur_position += free_count;

        ready_flag = true;
    }
    else {
        memcpy(p, data, length);
        detector_cur_position += length;
    }
}