#pragma once

#include <iostream>
#include <vector>
#include <string>

#ifdef __LINUX__
#include <ifaddrs.h>
#else
#include <winsock2.h>
#include <iphlpapi.h>
#endif

namespace cs
{
	class device_configuration
	{
	public:
		static bool get_config();
	private:
		static void print_ip_address(PIP_ADAPTER_UNICAST_ADDRESS pUnicast, std::vector<std::string>& addresses);
	};
}
