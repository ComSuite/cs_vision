/**
 * @file		GemmaDetector.cpp
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

#include "GemmaDetector.h"
#include "HTTPRequest.hpp"
#include "base64.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/rapidjson.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace cs;
using namespace std;
using namespace cv;
using namespace rapidjson;

GemmaDetector::GemmaDetector()
{
	// Initialize any member variables if needed
}

GemmaDetector::~GemmaDetector()
{
	clear();
}

int GemmaDetector::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
	// Initialize the detector with the provided model, label, and rules paths
	// Use GPU if is_use_gpu is true
	// Return 0 on success, or an error code on failure
	return 0;
}

int GemmaDetector::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu)
{
	model = model_path;
	prompt = input_tensor_name;
	endpoint = "http://192.168.1.130:11434/api/generate";

	try
	{
		http::Request request{ endpoint };

		Document root;

		root.SetObject();
		auto& allocator = root.GetAllocator();

		root.AddMember("model", Value().SetString(model.c_str(), model.length()), allocator);

		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		root.Accept(writer);
		const std::string json_string = buffer.GetString();

		const auto response = request.send("POST", json_string, {
			{"Content-Type", "application/json"}
			});

		std::string json_resp = std::string{ response.body.begin(), response.body.end() };
		cout << "Response: " << json_resp << endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Request failed, error: " << e.what() << '\n';
	}
}

void GemmaDetector::clear()
{
	// Clear any resources allocated by the detector
	// This may include freeing memory, closing files, etc.
}

int GemmaDetector::detect(cv::Mat* input, int& current_id, bool is_draw)
{
	try
	{
		http::Request request{ endpoint };

		Document root;

		root.SetObject();
		auto& allocator = root.GetAllocator();

		root.AddMember("model", Value().SetString(model.c_str(), model.length()), allocator);
		root.AddMember("prompt", Value().SetString(prompt.c_str(), prompt.length()), allocator);
		root.AddMember("stream", false, allocator);

		cv::Mat img; 
		cv::resize(*input, img, cv::Size(), 0.30, 0.30);

		std::vector<uchar> buf;
		cv::imencode(".jpg", img, buf);
		std::string encoded = base64_encode(buf.data(), buf.size());
		Value image_array(kArrayType);
		image_array.PushBack(Value().SetString(encoded.c_str(), encoded.length()), allocator);

		root.AddMember("images", image_array, root.GetAllocator());

		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		root.Accept(writer);
		const std::string json_string = buffer.GetString();

		const auto response = request.send("POST", json_string, {
			{"Content-Type", "application/json"}
			});

		std::string json_resp = std::string{ response.body.begin(), response.body.end() };

		try {
			if (!root.Parse(json_resp.c_str()).HasParseError()) {
				if (root.HasMember("response")) {
					if (root["response"].IsString()) {
						cout << "Response: " << root["response"].GetString() << endl;
					}
				}
			}
		}
		catch (...) {

		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Request failed, error: " << e.what() << '\n';
	}

	return 0;
}

int GemmaDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
	return 0;
}

int GemmaDetector::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	return 0;
}
