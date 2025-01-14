/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2021, Alexander Epstine (a@epstine.com)
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

#include "SimpleGStreamerVideoStreamer.h"
#include <iostream>

using namespace std;
using namespace cv;
using namespace cs;

int SimpleGStreamerVideoStreamer::open(int port)
{
    //gst - launch - 1.0 nvarguscamerasrc !'video/x-raw(memory:NVMM),width=1920, height=1080, framerate=30/1, format=NV12' !nvvidconv flip - method = 2 !nvv4l2h264enc insert - sps - pps = true bitrate = 16000000 !rtph264pay !udpsink port = 5000 host = $HOST
    cv::String outUrl = "gst-launch-1.0 -v v4l2src ! video/x-raw,format=YUY2,width=640,height=640 ! jpegenc ! rtpjpegpay ! udpsink port=" + to_string(port);
    writer.open(outUrl.c_str(), CAP_GSTREAMER, 0, 15.0, Size(640, 480));
    if (!writer.isOpened()) {
        cout << "[SimpleGStreamerVideoStreamer] Cannot open stream" << endl;
        return 0;
    }

    return 1;
}

void SimpleGStreamerVideoStreamer::show_frame(cv::Mat& frame, const char* channel)
{
    writer.write(frame);
}

