#include "device_configuration.h"
#include <iostream>
#include <cstring>

using namespace cs;

#ifdef __LINUX__
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

bool device_configuration::get_config()
{
    struct ifaddrs* interfaces = nullptr, * ifa = nullptr;

    if (getifaddrs(&interfaces) == -1) {
        perror("getifaddrs");
        return false;
    }

    for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) { 
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ip, INET_ADDRSTRLEN);
            std::cout << "Interface: " << ifa->ifa_name << " | IPv4 Address: " << ip << std::endl;
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6) {
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr, ip, INET6_ADDRSTRLEN);
            std::cout << "Interface: " << ifa->ifa_name << " | IPv6 Address: " << ip << std::endl;
        }
    }

    freeifaddrs(interfaces);
	return true;
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <WS2tcpip.h>

#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void device_configuration::print_ip_address(PIP_ADAPTER_UNICAST_ADDRESS pUnicast, std::vector<std::string>& addresses)
{
    char address[INET6_ADDRSTRLEN];

    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
        struct sockaddr_in* sa_in = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
        inet_ntop(AF_INET, &sa_in->sin_addr, address, sizeof(address));
    }
    else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
        struct sockaddr_in6* sa_in6 = (struct sockaddr_in6*)pUnicast->Address.lpSockaddr;
        inet_ntop(AF_INET6, &sa_in6->sin6_addr, address, sizeof(address));
    }

	addresses.push_back(address);
    printf("\tIP Address: %s\n", address);
}

void device_configuration::get_ip_addresses(std::vector<std::string>& addresses)
{
    DWORD dwRetVal = 0;
    unsigned int i = 0;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES*)MALLOC(outBufLen);
        if (pAddresses == NULL) {
            return;
        }

        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            if ((pCurrAddresses->IfType == IF_TYPE_IEEE80211 || pCurrAddresses->IfType == IF_TYPE_ETHERNET_CSMACD) && pCurrAddresses->PhysicalAddressLength > 0) {
                if (pCurrAddresses->OperStatus == IfOperStatusUp &&
                    !wcsstr(pCurrAddresses->Description, L"Virtual") &&
                    !wcsstr(pCurrAddresses->Description, L"VMware") &&
                    !wcsstr(pCurrAddresses->Description, L"Loopback") &&
                    !wcsstr(pCurrAddresses->Description, L"Bluetooth")
                    ) {
                    pUnicast = pCurrAddresses->FirstUnicastAddress;
                    while (pUnicast != NULL) {
                        print_ip_address(pUnicast, addresses);
                        pUnicast = pUnicast->Next;
                    }
                }
            }

            pCurrAddresses = pCurrAddresses->Next;
        }
    }

    if (pAddresses) {
        FREE(pAddresses);
    }
}

bool device_configuration::get_config()
{
    std::vector<std::string> addresses;

    get_ip_addresses(addresses);

    return true;
}
#endif


