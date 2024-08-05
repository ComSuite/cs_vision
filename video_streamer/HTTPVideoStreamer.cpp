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

#include "HTTPVideoStreamer.h"

using namespace std;
using namespace cv;
using namespace cs;
using namespace nadjieb;

HTTPVideoStreamer::HTTPVideoStreamer()
{
}

HTTPVideoStreamer::~HTTPVideoStreamer()
{

}

int HTTPVideoStreamer::open(int port)
{
    int ret = 0;

    try {
        streamer = new MJPEGStreamer();
        if (streamer) {
            streamer->start(port, 1);

            ret = 1;
        }
    }
    catch(...) {
        ret = 0;
    }

    return ret;
}

void HTTPVideoStreamer::show_frame(Mat& frame, const char* channel)
{
    if (streamer != NULL) {
        vector<uchar> buff_bgr;
        if (imencode(".jpg", frame, buff_bgr, params)) {
            streamer->publish(channel, string(buff_bgr.begin(), buff_bgr.end()));
        }
    }
}
