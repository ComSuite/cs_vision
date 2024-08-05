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

#include <cstdio>
#include <iostream>
#include <map>

#ifndef __LINUX__
#ifndef __int128_t
using __int128_t = int64_t;
#endif
#endif

namespace cs
{
	#define default_kpi_counter_step 50

	class kpi_counter
	{
	public:
		virtual void tick(int _count, int _key = 1, const char* prompt = nullptr)
		{
			key = key + _key;
			counter = counter + _count;
			distribution[_count]++;

			if (ticker == default_kpi_counter_step) {
				ticker = 0;
				//draw();
				/*
				for (auto n : distribution) {
					if (n.second > 0)
						std::cout << n.first << ":" << n.second << " ";
					else
						std::cout << "   " << " ";
				}
				*/
				std::cout << std::endl << std::endl;

				int n = counter / key;
				if (prompt != nullptr)
					std::cout << prompt << n << std::endl;
				else
					std::cout << "KPI: " << n << std::endl;
			}
			else
				ticker++;
		}

		void init(__uint128_t step = default_kpi_counter_step)
		{
			this->step = step;
			counter = 0;
			ticker = 0;
			key = 0;

			distribution.clear();
		}
	private:
		void draw()
		{
			const int chart_height = 10;
			std::map<int, int>::iterator best = std::max_element(distribution.begin(), distribution.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b)->bool { return a.second < b.second; });

			const int max_count = best->second;
			for (int current_height = chart_height; current_height > 0; --current_height) {
				for (auto item : distribution) {
					const int bar_height = (item.second * chart_height) / max_count;
					if (bar_height < current_height)
						std::cout << "     "; // we're still above the bar
					else if (bar_height == current_height)
						std::cout << "  _  "; // reached the top of the bar
					else // bar_height > current_height
						std::cout << " | | "; // now the rest of the bar ...
				}
				std::cout << '\n';
			}
		}

		__uint128_t ticker = 0;
		__uint128_t step = default_kpi_counter_step;
		__uint128_t key = 0;
		__uint128_t counter = 0;
		std::map<int, int> distribution;
	};
}
