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

#include "credentials_storage.h"
#include "std_utils.h"
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory_resource>
#include <string>
#include <string_view>


using namespace std;
using namespace cs;

#define STORAGE_NAME ".credentials"

std::string credentials_defaults::login = "admin";
std::string credentials_defaults::password = "admin";
int credentials_defaults::group_id = static_cast<int>(cs::groups::group_admins);
uint64_t credentials_defaults::acl = static_cast<uint64_t>(0);

credentials_storage::credentials_storage()
{
	if (!is_file_exists(STORAGE_NAME))
	{
		if (add(credentials_defaults::login, credentials_defaults::password)) {
			save();
		}
	}

	load(STORAGE_NAME);
}

credentials_storage::~credentials_storage()
{
	::clear(credentials);
}

bool credentials_storage::check(const std::string& login, const std::string& password)
{
	credential* cred = credentials.at(login);
	if (cred != nullptr) {
		auto h = std::hash<std::string>{}(password);
		return cred->hash == h;
	}

	return false;
}

bool credentials_storage::change(const std::string& login, const std::string& password)
{
	credential* cred = credentials.at(login);
	if (cred != nullptr) {
		auto h = std::hash<std::string>{}(password);
		cred->hash = h;

		return save();
	}

	return false;
}

bool credentials_storage::add(const std::string& login, const std::string& password)
{
	credential* cred = credentials.at(login);
	if (cred == nullptr) {
		cred = new credential();
		if (cred != nullptr) {
			cred->hash = std::hash<std::string>{}(password);
			credentials[login] = cred;
		}

		return true;
	}

	return false;
}

bool credentials_storage::del(const std::string& login)
{
	credential* cred = credentials.at(login);
	if (cred != nullptr) {
		delete cred;
		credentials.erase(login);

		return true;
	}

	return false;
}

bool credentials_storage::save()
{

}

int credentials_storage::parse(rapidjson::Document& root)
{
	::clear(credentials);

	try {
		if (root.IsNull())
			return 0;

		if (!root.IsArray())
			return 0;

		auto _credentials = root.GetArray();
		for (auto& _cred : _credentials) {
			auto cred = new credential();
			if (cred != nullptr) {
				cred->hash = static_cast<size_t>(json_get_int(_cred, "pwdhash", 0));
				cred->group_id = json_get_int(_cred, "groups", 0);
				cred->acl = json_get_int(_cred, "acl", 0);

				string login = json_get_string(_cred, "login", 0);
				credentials[login] = cred;
			}
		}
	}
	catch (...) {
		return 0;
	}

	return 1;
}


