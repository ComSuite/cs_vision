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

#include "JsonValidator.h"

using namespace std;
using namespace cs;
using namespace rapidjson;

bool JsonValidator::validate(const char* src, const char* templ, bool verbose)
{
	is_template = true;
	if (!JsonWrapper::parse(templ))
		return false;

	is_template = false;
	if (!JsonWrapper::parse(src))
		return false;


}

int JsonValidator::parse(rapidjson::Document& root)
{
	if (is_template) {
		//template_root = root;
		return 1;
	}

	for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it)
	{
		//parseRecursive("", it, _values);
	}
}