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

#include "system_usage.h"
#include "linux-system-usage.hpp"

using namespace cs;

void system_usage::init()
{
}

unsigned int system_usage::get_free_memory()
{
	auto memory_data = get_system_usage_linux::read_memory_data();
	auto memory_usage = 100.0f * memory_data.get_memory_usage();
	return static_cast<int>(100 - memory_usage);
}

get_system_usage_linux::CPU_stats prev_cpu_data;
bool is_prev = false;
unsigned int system_usage::get_cpu_usage()
{
	if (!is_prev)
	{
		prev_cpu_data = get_system_usage_linux::read_cpu_data();
		is_prev = true;
		return 0;
	}

	auto cpu_data = get_system_usage_linux::read_cpu_data();
	auto cpu = 100.0f * get_system_usage_linux::get_cpu_usage(prev_cpu_data, cpu_data);
	prev_cpu_data = cpu_data;

	return static_cast<int>(cpu);
}

int system_usage::get_cpu_temp()
{
	double temp = get_system_usage_linux::get_thermalzone_temperature(0);
	return static_cast<int>(temp / 1000);
}

unsigned int system_usage::get_gpu_usage()
{
	return get_system_usage_linux::read_gpu_load();
}