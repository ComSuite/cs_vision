/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2024, Alexander Epstine (a@epstine.com)
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

#include <iostream>
#include "settings.h"
#include "mongoose_loop.h"
#include <pthread.h>

using namespace std;
using namespace cs;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "\nError! Usage: <path to settings file>" << endl << endl;
        return 1;
    }

    device_settings settings;
    if (!settings.load(argv[1])) {
        cout << "Invalid settings file: <" << argv[1] << ">!" << endl;
        return 1;
    }

    settings.http_server->device_name = settings.name;
    http_server_thread_arg* http_arg = new http_server_thread_arg();
    if (http_arg != nullptr) {
        http_arg->settings_file_path = settings.get_file_path();
        http_arg->settings = settings.http_server;
        http_arg->queue = new BaseQueue<fps_counter_info>();
    }

    pthread_t http_thread;
    if (http_arg != nullptr) {
        http_arg->camera_count = settings.cameras.size();

        pthread_create(&http_thread, NULL, mongoose_thread_func, http_arg);
        pthread_detach(http_thread);
    }

    unsigned int current = 10;
    while (true) {
        fps_counter_info* info = new fps_counter_info();
        info->counter = 30 - current;
        current = info->counter;
        info->id = "1";
        http_arg->queue->push(info);
        info->id = "2";
        info->counter += 5;
        http_arg->queue->push(info);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
