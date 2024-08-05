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

#pragma once

#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#ifdef __WITH_FILESYSTEM_CXX__
#include <filesystem>
#endif
#include "std_utils.h"

namespace cs
{
	class ReadOnlyValues
	{
	public:
		ReadOnlyValues(const char* dictionary_path) 
		{
			load(dictionary_path);
		}

		bool check(std::string& path)
		{
			std::list<std::string>::iterator iter = std::find(patches.begin(), patches.end(), path);
			return iter == patches.end();
		}

		bool load(const char* filename)
		{
			patches.clear();
			return txt_load_to<std::list>(filename, patches);
/*
			try {
				std::ifstream f(filename);

				if (!f) {
					std::cerr << "ERROR: Cannot open " << filename << std::endl;
					return false;
				}

				std::string line;
				while (std::getline(f, line)) {
					patches.push_back(trim(line));
				}
			}
			catch (const std::exception& ex) {
				std::cerr << "Exception: '" << ex.what() << std::endl;
			}
			return true;
*/
		}
	private:
		std::list<std::string> patches;
	};
}
