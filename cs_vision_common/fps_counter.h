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

#include <chrono>
#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <cstdint>
#include <string>
#include "BaseQueue.h"

#ifndef __LINUX__
#ifndef __int128_t
using __int128_t = int64_t;
#endif
#ifndef __uint128_t
using __uint128_t = uint64_t;
#endif
#endif

namespace cs
{
	#define default_fps_counter_step 50

	class fps_counter_info
	{
	public:
		std::string id = "";
		unsigned int counter = 0;
	};

	class fps_counter
	{
	public:
		void tick(const char* prompt, const char* id, BaseQueue<fps_counter_info>* queue = nullptr)
		{
			if (!is_init)
				return;

			counter++;
			if (ticker == default_fps_counter_step) {
				ticker = 0;
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
				if (diff != 0) {
					unsigned int val = static_cast<unsigned int>(counter / diff);
					if (queue != nullptr) {
						fps_counter_info* info = new fps_counter_info();
						if (info != nullptr) {
							info->counter = val;
							info->id = id;

							queue->try_push(info);
						}
					}

#ifdef _DEBUG
					std::cout << prompt << val << std::endl;
#endif
				}
			}
			else
				ticker++;
		}

		void init(unsigned int step = default_fps_counter_step)
		{
			this->step = step;
			counter = 0;
			ticker = 0;
			begin = std::chrono::steady_clock::now();
			is_init = true;
		}
	private:
		unsigned int step = default_fps_counter_step;
		__int128_t counter = 0;
		std::chrono::steady_clock::time_point begin;
		int ticker = 0;
		bool is_init = false;
	};
}
