#pragma once

namespace cs
{
	class system_usage
	{
	public:
		void init();
		unsigned int get_cpu_usage();
		unsigned int get_free_memory();
	private:
	};
}

