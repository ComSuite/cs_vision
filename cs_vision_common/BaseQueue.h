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

#pragma once

#include <queue>
#include <mutex>

namespace cs
{
    template<class T>
    class BaseQueue
    {
    public:
        BaseQueue() {}

        void push(T* elem) {
            if (elem == nullptr)
                return;

            m.lock();
            q.push(elem);
            m.unlock();
        }

        T* pop()
        {
            T* elem = nullptr;

            m.lock();
            if (!q.empty()) {
                elem = q.front();
                q.pop();
            }
            m.unlock();

            return elem;
        }
    private:
        std::queue<T*> q;
        std::mutex m;
    };
}

