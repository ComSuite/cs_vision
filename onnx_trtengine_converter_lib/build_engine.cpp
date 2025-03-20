/**
 * @file
 * 
 * @author      Noah van der Meer
 * @brief       YoloV5-TensorRT example: build a TensorRT engine
 * 
 * Copyright (c) 2021, Noah van der Meer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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
 * 
 */

//#include "yolov5_builder.hpp"
//#include "engine_information.h"
#include <iostream>
#include <algorithm>
#include <string>
#include "cxxopts.hpp"
#include "yolov11.h"

using namespace std;

class builder_params
{
public:
    string model_file_path = "";
    string engine_file_path = "";
    string precision = "fp32";
};

void parse_command_line(int argc, char* argv[], builder_params* param)
{
    if (param == nullptr)
        return;

    cxxopts::Options options("", "");
    options.allow_unrecognised_options();
    options.add_options()
        ("h,help", "show this help menu")
        ("m,model", "[mandatory] specify the ONNX model file", cxxopts::value<string>())
        ("o,output", "[mandatory] specify the engine output", cxxopts::value<string>())
        ("p,precision", "[optional] specify the precision. Options: fp32, fp16", cxxopts::value<string>());

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << endl;
        exit(0);
    }

    if (result.count("model")) {
        param->model_file_path = result["model"].as<string>();
    }
    else {
        cout << options.help() << endl;
        exit(0);
    }

    if (result.count("output")) {
        param->engine_file_path = result["output"].as<string>();
    }
    else {
        cout << options.help() << endl;
        exit(0);
    }

    if (result.count("precision")) {
        param->precision = result["precision"].as<string>();
    }
}

int main(int argc, char* argv[])
{
    builder_params param;
    parse_command_line(argc, argv, &param);

    class TRTYOLOv11Logger : public nvinfer1::ILogger {
        void log(Severity severity, const char* msg) noexcept override {
            if (severity <= Severity::kVERBOSE)
                std::cout << msg << std::endl;
        }
    } trt_logger;


	YOLOv11 detector;
	detector.build(param.model_file_path, trt_logger);
	print_engine_info(detector.get_engine());
	detector.save_engine(param.engine_file_path);

	return 0;
}

