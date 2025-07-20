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

#include "TFYOLOv5ObjectDetector.h"
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <exception>
#include <iterator>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/core/mat.hpp>

#ifdef __HAS_CUDA__
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudawarping.hpp>
#endif

#include <tensorflow/lite/c/c_api.h>
#include "std_utils.h"

using namespace std;
using namespace cv;
using namespace cs;
using namespace rapidjson;

TFYOLOv5ObjectDetector::TFYOLOv5ObjectDetector()
{
    //cout << "TFLite version: " << TfLiteVersion() << endl;
}

TFYOLOv5ObjectDetector::~TFYOLOv5ObjectDetector()
{
    clear();
}

int TFYOLOv5ObjectDetector::init(object_detector_environment& env)
{
    yolo_model.confThreshold = 0.30;
    yolo_model.nmsThreshold = 0.40;
    yolo_model.nthreads = 4;

    yolo_model.loadModel(env.model_path);
    yolo_model.getLabelsName(env.label_path, labels);

    cout << "[TFYOLOv5ObjectDetector] Label Count: " << labels.size() << "\n" << endl;

    load_rules(env.rules_path.c_str());

    width = yolo_model._in_width;
    height = yolo_model._in_height;

    return 1;
}

void TFYOLOv5ObjectDetector::clear()
{

}

int TFYOLOv5ObjectDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
    if (input == nullptr)
        return 0;

    cv::Mat image;
    input->download(image);

    return detect(&image, current_id, is_draw);
}

int TFYOLOv5ObjectDetector::detect(Mat* image, int& current_id, bool is_draw)
{
    if (!image)
        return 0;

    Mat detect;
    image->copyTo(detect);

    yolo_model.run(detect, out_pred);

    auto boxes = out_pred.boxes;
    auto scores = out_pred.scores;
    auto _labels = out_pred.labels;
    clear_last_detections();

    for (int i = 0; i < boxes.size(); i++)
    {
        auto ind = _labels[i];
        auto box = boxes[i];
        auto score = scores[i];

        DetectionItem* item = new DetectionItem();
        item->color = color;
        if (check_rule(ind, score, item->color)) {
            DetectionItem* item = new DetectionItem();
            item->id = current_id;
            item->detector_id = id;
            current_id++;
            item->kind = ObjectDetectorKind::OBJECT_DETECTOR_TENSORFLOW_YOLOv5;
            item->label = trim(labels[ind]);
            item->class_id = ind;
            item->score = score;
            item->box.x = box.x;
            item->box.y = box.y;
            item->box.width = box.width;
            item->box.height = box.height;
            item->neural_network_id = neural_network_id;

            last_detections.push_back(item);

            if (is_draw) {
                cv::rectangle(*image, box, cv::Scalar(255, 0, 0), 2);
                cv::putText(*image, labels[ind], cv::Point(box.x, box.y), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
            }
        }
        else
            delete item;
    }

    out_pred = {};

    return 1;
}

