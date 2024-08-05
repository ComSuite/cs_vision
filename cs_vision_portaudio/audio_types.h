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

#pragma once

#include <atomic>
#include <map>
#include "portaudio.h"
#include <mutex>
#include <thread>

namespace cs
{
    enum class FILTER_KIND
    {
        FILTER_NONE = 0,
        FILTER_LOW_PASS = 1,
        FILTER_HIGH_PASS = 2,
        FILTER_NOTCH = 3
    };

    class audio_callback_data
    {
    public:
        audio_callback_data()
        {
        }

        virtual ~audio_callback_data()
        {
        }

        float volume_gain = 1.0;

        unsigned long frames_per_buffer = 0;
        int num_channels = 0;
        PaTime start_time = 0;
        PaTime duration = 0;

        void* microphone = nullptr;
    };
}
