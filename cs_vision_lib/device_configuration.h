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
		static const size_t INET6_ADDRSTRLEN = 46;

		static void get_ip_addresses(std::vector<std::string>& addresses);
#ifdef __LINUX__
		static void print_ip_address(struct ifaddrs* ifa, std::vector<std::string>& addresses);
#else
		static void print_ip_address(PIP_ADAPTER_UNICAST_ADDRESS pUnicast, std::vector<std::string>& addresses);
#endif
	};
}
