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
#include <windows.h>
#include <pdh.h>
#include <cstdio>
#include <iostream>
#ifdef __HAS_CUDA__
#include <nvml.h>
#endif

using namespace cs;
using namespace std;

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void system_usage::init()
{
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

unsigned int system_usage::get_free_memory()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex))
		std::cout << GetLastError() << std::endl;

	return statex.dwMemoryLoad;
}

unsigned int system_usage::get_cpu_usage()
{
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return static_cast<unsigned int>(counterVal.doubleValue);
}

int system_usage::get_cpu_temp()
{
	return 0;
}

unsigned int system_usage::get_gpu_usage()
{
#ifdef __HAS_CUDA__ 
#ifdef __WITH_NVML__
	nvmlInit();
	nvmlDevice_t device;
	nvmlDeviceGetHandleByIndex(0, &device);

	nvmlUtilization_t utilization;
	nvmlDeviceGetUtilizationRates(device, &utilization);
	nvmlShutdown();

	return static_cast<unsigned int>(utilization.gpu);
	//std::cout << "Memory Utilization: " << utilization.memory << "%" << std::endl;
#endif
#endif
	return 0;
}
