/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2021, Alexander Epstine (a@epstine.com)
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

#include <map>
#include <list>
#include <type_traits>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>

template <typename T1, typename T2>
T1 nearest_key(const std::map<T1, T2>& data, T1 key)
{
	if (data.size() == 0) {
		throw std::out_of_range("Received empty map");
	}

	auto lower = data.lower_bound(key);

	if (lower == data.end())
		return std::prev(lower)->first;

	if (lower == data.begin())
		return lower->first;

	auto previous = std::prev(lower);
	if ((key - previous->first) < (lower->first - key))
		return previous->first;

	return lower->first;
}

template<typename T, template<class> class C>
void clear(C<T*>& lst)
{
	while (!lst.empty()) {
		if (lst.front() != nullptr)
			delete lst.front();
		lst.pop_front();
	}
}

template<typename T, typename C>
void clear(std::map<T, C*>& lst)
{
	for (auto i : lst) {
		if (i.second != nullptr)
			delete i.second;
	}

	lst.clear();
}

template <typename T>
constexpr auto to_underlying(T t) noexcept
{
	return static_cast<std::underlying_type_t<T>>(t);
}

std::string ltrim(const std::string& s);
std::string rtrim(const std::string& s);
std::string trim(const std::string& s);

template<template <class> class T>
bool txt_load_to(const char* file_path, T<std::string>& out)
{
	try {
		std::ifstream f(file_path);

		if (!f) {
			std::cerr << "ERROR: Cannot open " << file_path << std::endl;
			return false;
		}

		std::string line;
		while (std::getline(f, line)) {
			out.push_back(trim(line));
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "Exception: '" << ex.what() << std::endl;
	}

	return true;
}

std::string read_str_file(const char* filename);
bool write_str_file(const char* filename, const char* content);

bool is_file_exists(const char* filename);
