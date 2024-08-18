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
#include "../rapidjson/writer.h"

using namespace std;
using namespace cs;
using namespace rapidjson;

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
	if (login.size() <= 0 || password.size() <= 0)
		return false;

	try {
		credential* cred = credentials.at(login);
		if (cred != nullptr) {
			auto h = std::hash<std::string>{}(password);
			return cred->hash == h;
		}
	}
	catch (...) {

	}

	return false;
}

bool credentials_storage::check_token(const char* token, char* login, size_t length)
{
	try {
		auto it = tokens.at(token);
		if (login != NULL && it.size() < length)
			strncpy(login, it.c_str(), length);
		return true;
	}
	catch (...) {

	}

	return false;
}

bool credentials_storage::add_token(const char* token, const char* login)
{
	if (!check_token(token, NULL, 0)) {
		tokens[token] = login;
		return true;
	}

	return false;
}

bool credentials_storage::del_token(const char* token)
{
	for (auto i : tokens)
		cout << i.first << "    " << i.second << endl;
	if (check_token(token, NULL, 0)) {
		tokens.erase(token);
		return true;
	}

	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << tokens.size() << endl;

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
	try {
		if (credentials.find(login) == credentials.end()) {
			credential* cred = new credential();
			if (cred != nullptr) {
				cred->hash = std::hash<std::string>{}(password);
				credentials[login] = cred;
			}

			return true;
		}
	}
	catch (...) {

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
	Document root;
	root.SetArray();
	auto& allocator = root.GetAllocator();

	for (auto& it : credentials) {
		Document object;
		object.SetObject();

		object.AddMember("login", Value().SetString(it.first.c_str(), it.first.length(), allocator), allocator);
		object.AddMember("pwdhash", it.second->hash, allocator);
		object.AddMember("groups", it.second->group_id, allocator);
		object.AddMember("acl", it.second->acl, allocator);

		root.PushBack(object, allocator);
	}

	StringBuffer buffer;
	buffer.Clear();

	Writer<StringBuffer> writer(buffer);
	root.Accept(writer);

	const char* payload = buffer.GetString();
	if (payload == NULL || strlen(payload) == 0)
		return false;

	return write_str_file(STORAGE_NAME, payload);
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
				cred->hash = json_get_uint64(_cred, "pwdhash", 0);
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


