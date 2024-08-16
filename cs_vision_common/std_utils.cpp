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

#include <string>
#include <vector>
#include <ios>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

#ifdef __WITH_FILESYSTEM_CXX__
#include <filesystem>
#endif

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string& s)
{
	return rtrim(ltrim(s));
}

std::string read_str_file(const char* filename)
{
    std::ifstream ifs(filename, std::ios::in | std::ios::binary | std::ios::ate);

    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
}

bool write_str_file(const char* filename, const char* content)
{
	try {
		std::ofstream out(filename);
		out << content;
		out.close();
	}
	catch (...) {
		return false;
	}

	return true;
}

bool is_file_exists(const char* filename)
{
#ifdef __WITH_FILESYSTEM_CXX__
	return std::filesystem::exists(std::filesystem::path(filename));
#else
	std::ifstream ifs;
	ifs.open(filename);

	if (ifs.is_open()) {
		ifs.close();
		return true;
	}
#endif
	return false;
}
