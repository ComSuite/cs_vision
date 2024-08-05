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

#include "FileBackup.h"

#ifdef __WITH_FILESYSTEM_CXX__
#include <filesystem>
#endif
#include <fstream>
#include <iostream>

using namespace std;
#ifdef __cpp_lib_filesystem
using namespace std::filesystem;
#endif
using namespace cs;

std::string FileBackup::default_backup_extension = "bak";

bool FileBackup::create_backup(const char* source, const char* extension)
{
#ifdef __cpp_lib_filesystem
	path dest = source;

	dest.replace_extension(extension);
	return std::filesystem::copy_file(source, dest, copy_options::overwrite_existing);
#endif
	return false;
}

bool FileBackup::restore(const char* source, const char* extension)
{
#ifdef __cpp_lib_filesystem
	path dest = source;

	dest.replace_extension(extension);
	return std::filesystem::copy_file(dest, source, copy_options::overwrite_existing);
#endif
	return false;
}
