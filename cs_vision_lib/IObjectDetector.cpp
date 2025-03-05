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

#include "IObjectDetector.h"
#include <iostream>
#include <fstream>
#ifdef __cpp_lib_filesystem
#include <filesystem>
#endif
#include <string>
#include <list>
#include <exception>
#include <iterator>
#include <algorithm>
#include "std_utils.h"
#include "JsonCOCOLabels.h"
#include <cv_utils.h>

#ifdef __HAS_CUDA__
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudawarping.hpp>
#endif

using namespace std;
using namespace cs;
using namespace cv;
using namespace cv::cuda;
using namespace rapidjson;

void default_error_reporter(void* user_data, const char* format, va_list args)
{
    std::cout << "Detector Error!!!" << std::endl;
}

int IObjectDetector::infer(cv::Mat* input, int& current_id, bool show_mean, bool is_draw)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int ret = detect(input, current_id, is_draw);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    detect_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    detections_count++;

    if (show_mean) {
        cout << get_mean_detect_duration() << endl;
    }

    return ret;
}

float IObjectDetector::get_mean_detect_duration()
{
    if (detections_count != 0)
        return detect_time / detections_count;
    else
        return 0;
}

void IObjectDetector::ProcessInputWithFloatModel(uint8_t* input, float* buffer, const int width, const int height, const int channels)
{
    for (int y = 0; y < height; ++y) {
        float* out_row = buffer + (y * width * channels);
        for (int x = 0; x < width; ++x) {
            uint8_t* input_pixel = input + (y * width * channels) + (x * channels);
            float* out_pixel = out_row + (x * channels);
            for (int c = 0; c < channels; ++c) {
                out_pixel[c] = input_pixel[c]; // / 255.0f
            }
        }
    }
}

cv::Mat IObjectDetector::ProcessOutputWithFloatModel(float* input, const int width, const int height, const int channels)
{
    cv::Mat image = cv::Mat::zeros(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float* input_pixel = input + (y * width * channels) + (x * channels);
            cv::Vec3b& color = image.at<cv::Vec3b>(cv::Point(x, y));
            color[0] = (uchar)floor(input_pixel[0] * 255.0f);
            color[1] = (uchar)floor(input_pixel[1] * 255.0f);
            color[2] = (uchar)floor(input_pixel[2] * 255.0f);
        }
    }

    return image;
}

bool IObjectDetector::if_json(const char* label_path)
{
#ifdef __cpp_lib_filesystem
    filesystem::path path(label_path);

    string ext = path.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext.compare(".json") == 0 || ext.compare(".jsn") == 0)
        return true;
    else
        return false;
#else
    string file_name(label_path);

    if (file_name.find_last_of(".") != std::string::npos) {
        string ext = file_name.substr(file_name.find_last_of("."));
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext.compare(".json") == 0 || ext.compare(".jsn") == 0)
            return true;
    }

    return false;
#endif
}

void IObjectDetector::load_labels_txt(const char* label_path)
{
    labels.clear();

    try {
        ifstream f(label_path);

        if (!f) {
            cerr << "ERROR: Cannot open " << label_path << endl;
            return;
        }

        string line;
        while (getline(f, line)) {
            labels.push_back(trim(line));
        }
    }
    catch (const std::exception& ex) {
        cerr << "Exception: '" << ex.what() << "'!" << endl;
    }
}

void IObjectDetector::load_labels_coco_json(const char* label_path)
{
    labels.clear();

    JsonCOCOLabels* json = new JsonCOCOLabels();
    if (json != nullptr) {
        json->load(label_path);
        json->get(labels); 

        delete(json);
    }
}

void IObjectDetector::load_labels(const char* label_path)
{
    if (if_json(label_path))
        load_labels_coco_json(label_path);
    else
        load_labels_txt(label_path);
}

void IObjectDetector::load_rules(const char* rules_path)
{
    ::clear<int, DetectionRule>(rules);
    load(rules_path);
}

DetectionRule* IObjectDetector::get_rule(int ind)
{
    DetectionRule* rule = nullptr;

    try {
        if (rules.find(ind) != rules.end()) {
            rule = rules[ind];
        }
    }
    catch (...) {
        rule = nullptr;
    }

    return rule;
}

bool IObjectDetector::get_rule_label(int ind, string& label)
{
    if (labels.size() == 0 || ind > labels.size())
        return false;

    bool ret = true;

    try {
        label = trim(labels[ind]);
    }
    catch (...) {
        label = "";
        ret = false;
    }

    return ret;
}

float IObjectDetector::get_rule_score(int ind)
{
    float score = 1;
    DetectionRule* rule = get_rule(ind);

    if (rule != nullptr) {
        score = rule->score;
    }

    return score;
}

bool IObjectDetector::check_rule(int ind, float score, cv::Scalar& color)
{
    DetectionRule* rule = get_rule(ind);
    if (rule == nullptr)
        return true;

    color = rule->color;
    if (score >= rule->score)
        return true;
    else
        return false;
}

int IObjectDetector::parse(Document& root)
{
    int ret = 1;

    try {
        if (root.IsNull())
            return 0;

        if (!root.HasMember("rules"))
            return 0;

        if (!root["rules"].IsArray())
            return 0;

        auto _rules = root["rules"].GetArray();
        for (auto& _rule : _rules) {
            DetectionRule* rule = new DetectionRule();
            int ind = json_get_int(_rule, "id", -1);
            rule->object = ind;
            rule->description = json_get_string(_rule, "description", "");
            rule->on_detect = json_get_string(_rule, "on_detect", "");
            rule->execute_always = json_get_bool(_rule, "execute_always", false);
            rule->score = json_get_double(_rule, "score", 1);
            rule->color = cv_string_to_color(json_get_string(_rule, "color", "0x00FFFFFF"));

            rules[ind] = rule;
        }
    }
    catch (...) {
        ret = 0;
    }

    return ret;
}

//#ifdef __HAS_CUDA__
//void IObjectDetector::draw_detection(cv::cuda::GpuMat* detect_frame, DetectionItem* detection, cv::Scalar& background_color, bool is_show_mask)
//#else
void IObjectDetector::draw_detection(cv::Mat* detect_frame, DetectionItem* detection, cv::Scalar& background_color, bool is_show_mask)
//#endif
{
    if (is_show_mask) {
        draw_mask(detection, detect_frame, detection->color);
    }
    else {
#ifdef __HAS_CUDA__
        draw_mask(detection, detect_frame, detection->color);
#else
        rectangle(*detect_frame, static_cast<Rect>(detection->box), detection->color, 2, LINE_8);
        //draw_label(*env->detect_frame, det->label, det->box.x, det->box.y);
#endif
    }
}

//#ifdef __HAS_CUDA__
//void IObjectDetector::draw_mask(DetectionItem* det, cv::cuda::GpuMat* frame, const cv::Scalar& color)
//#else
void IObjectDetector::draw_mask(DetectionItem* det, cv::Mat* frame, const cv::Scalar color)
//#endif
{
    int x0 = (trunc(det->box.x) >= 0) ? trunc(det->box.x) : 0;
    int x1 = (x0 + trunc(det->box.width) > frame->cols) ? frame->cols : x0 + trunc(det->box.width);
    int y0 = (trunc(det->box.y) >= 0) ? trunc(det->box.y) : 0;
    int y1 = (y0 + trunc(det->box.height) > frame->rows) ? frame->rows : y0 + trunc(det->box.height);

//#ifdef __HAS_CUDA__
//    GpuMat sub = (*(frame))(Range(y0, y1), Range(x0, x1));
//    GpuMat clr(sub.size(), CV_8UC3, color);
//    if (sub.cols > 0 && sub.rows > 0) {
//        double alpha = 0.3;
//        cv::cuda::addWeighted(clr, alpha, sub, 1.0 - alpha, 0.0, sub);
//        (*(frame))(Range(y0, y1), Range(x0, x1)) = sub;
//    }
//#else
    Mat sub = (*(frame))(Range(y0, y1), Range(x0, x1));
    Mat clr(sub.size(), CV_8UC3, color);
    if (sub.cols > 0 && sub.rows > 0) {
        double alpha = 0.3;
        cv::addWeighted(clr, alpha, sub, 1.0 - alpha, 0.0, sub);
        (*(frame))(Range(y0, y1), Range(x0, x1)) = sub;
    }
//#endif
}

void IObjectDetector::draw_label(Mat& input_image, string& label, int left, int top, cv::Scalar& background_color, cv::Scalar& text_color)
{
    int base_line = 0;
    Size label_size = getTextSize(label, FONT_HERSHEY_COMPLEX, 0.7, 1, &base_line);
    top = max(top, label_size.height);
    Point tlc = Point(left, top);
    Point brc = Point(left + label_size.width, top + label_size.height + base_line);

    rectangle(input_image, tlc, brc, background_color, FILLED);
    putText(input_image, label.c_str(), Point(left, top + label_size.height), FONT_HERSHEY_COMPLEX, 0.7, text_color, 1, false);
}
