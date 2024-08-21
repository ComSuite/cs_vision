#pragma once

#ifdef __HAS_CUDA__
#include <nvml.h>
#endif

namespace cs
{
	class system_usage
	{
	public:
		void init();
		unsigned int get_cpu_usage();
		unsigned int get_free_memory();
		unsigned int get_gpu_usage()
		{
#ifdef __HAS_CUDA__
			nvmlInit();
			nvmlDevice_t device;
			nvmlDeviceGetHandleByIndex(0, &device);

			nvmlUtilization_t utilization;
			nvmlDeviceGetUtilizationRates(device, &utilization);
			nvmlShutdown();

			return static_cast<unsigned int>(utilization.gpu);
			//std::cout << "Memory Utilization: " << utilization.memory << "%" << std::endl;

#else
			return 0;
#endif
		}
	private:
	};
}

