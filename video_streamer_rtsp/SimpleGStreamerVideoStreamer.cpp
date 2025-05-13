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
#include <thread>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "std_utils.h"

using namespace std;
using namespace cv;
using namespace cs;

void SimpleGStreamerVideoStreamer::init(int port, const char* channel_name, int width, int height, int fps)
{
    gst_init(NULL, NULL);
    GMainLoop* serverloop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer* server = gst_rtsp_server_new();
    g_object_set(server, "service", std::to_string(port).c_str(), NULL);
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory* factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, "( udpsrc port=5004 ! application/x-rtp,encoding-name=H264 ! rtph264depay ! h264parse ! rtph264pay name=pay0 )");
    std::string channel = channel_name;
    channel = trim(channel);
    if (channel.at(0) != '/') {
        channel = "/" + channel;
    }
    gst_rtsp_mount_points_add_factory(mounts, channel.c_str(), factory);
    gst_rtsp_server_attach(server, NULL);
    std::thread serverloopthread(g_main_loop_run, serverloop);

    std::cout << "stream ready at rtsp://127.0.0.1:" << port << channel << std::endl;
    serverloopthread.detach();

    cv::String uri = "appsrc ! queue ! videoconvert ! video/x-raw,format=I420 ! x264enc key-int-max=30 insert-vui=1 tune=zerolatency ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5004";
    writer.open(uri.c_str(), CAP_GSTREAMER, 0, fps, Size(width, height), true);
    if (!writer.isOpened()) {
        cout << "[SimpleGStreamerVideoStreamer] Cannot open stream" << endl;
    }
}

int SimpleGStreamerVideoStreamer::open(int port, int tunneling_port)
{
    return 1;
}

void SimpleGStreamerVideoStreamer::show_frame(cv::Mat& frame, const char* channel)
{
    writer << frame;
}

