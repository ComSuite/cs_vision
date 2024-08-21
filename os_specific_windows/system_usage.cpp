#include "system_usage.h"

#include <windows.h>
#include <pdh.h>
#include <cstdio>
#include <iostream>

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
