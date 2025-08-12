/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2025, Alexander Epstine (a@epstine.com)
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

#include "device_configuration.h"
#include <iostream>
#include <cstring>
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/rapidjson.h"

using namespace cs;
using namespace rapidjson;

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

bool device_configuration::get_config(device_settings* device, std::string& config)
{
    std::vector<std::string> addresses;

    get_ip_addresses(addresses);

    Document root;

    root.SetObject();
    auto& allocator = root.GetAllocator();

    Value stream_array(kArrayType);
    for (const auto& stream : device->cameras) {
        Value stream_obj(kObjectType);
		stream_obj.AddMember("name", Value().SetString(stream->name.c_str(), stream->name.length()), allocator);
		stream_obj.AddMember("id", Value().SetString(stream->id.c_str(), stream->id.length()), allocator);
		stream_obj.AddMember("video_stream_port", stream->video_stream_port, allocator);
        stream_obj.AddMember("video_stream_channel", Value().SetString(stream->video_stream_channel.c_str(), stream->video_stream_channel.length()), allocator);
        stream_obj.AddMember("is_mqtt", stream->mqtt, allocator);
        stream_obj.AddMember("mqtt_broker_ip", Value().SetString(stream->mqtt_broker_ip.c_str(), stream->mqtt_broker_ip.length()), allocator);
        stream_obj.AddMember("mqtt_broker_port", stream->mqtt_broker_port, allocator);
        stream_obj.AddMember("mqtt_detection_topic", Value().SetString(stream->mqtt_detection_topic.c_str(), stream->mqtt_detection_topic.length()), allocator);
        stream_obj.AddMember("mqtt_is_send_empty", stream->mqtt_is_send_empty, allocator);

		stream_array.PushBack(stream_obj, allocator);
	}
	root.AddMember("video_streams", stream_array, allocator);

    Value addr_array(kArrayType);
    for (const auto& addr : addresses) {
        Value encoded;
        encoded.SetString(addr.c_str(), addr.length(), allocator);
        addr_array.PushBack(encoded, allocator);
	}
	root.AddMember("ip_addresses", addr_array, allocator);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    root.Accept(writer);
    config = buffer.GetString();

    return true;
}
#endif


