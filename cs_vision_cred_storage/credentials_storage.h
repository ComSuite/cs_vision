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

#include "JsonWrapper.h"
#include <map>

namespace cs
{
	enum class groups
	{
		group_none = 0,
		group_admins = 1,
		group_users = 2,
		group_guests = 3
	};

	class credentials_defaults
	{
	public:
		static std::string login;
		static std::string password;
		static int group_id;
		static uint64_t acl;
	};

	class credential
	{
	public:
		size_t hash = 0;
		int group_id = -1;
		uint64_t acl;
	};

	class credentials_storage : public JsonWrapper
	{
	public:
		credentials_storage();
		virtual ~credentials_storage();

		bool check(const std::string& login, const std::string& password);
		bool change(const std::string& login, const std::string& password);
		bool add(const std::string& login, const std::string& password);
		bool del(const std::string& login);

		bool save();
	private:
		std::map<std::string, cs::credential*> credentials;
	protected:
		int parse(rapidjson::Document& root) override;
	};
}

