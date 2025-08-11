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
