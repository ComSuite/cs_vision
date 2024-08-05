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

#include "yolov5_builder.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include "cxxopts.hpp"

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

    yolov5::Precision precision = yolov5::PRECISION_FP32;   /*  default */
    if(param.precision == "fp32") {
    }
    else if(param.precision == "fp16") {
        precision = yolov5::PRECISION_FP16;
    }
    else {
        std::cout << "Invalid precision specified: " << param.precision << std::endl;
        return 1;
    }

    yolov5::Builder builder;

    yolov5::Result r = builder.init();
    if(r != yolov5::RESULT_SUCCESS) {
        std::cout << "init() failed: " << yolov5::result_to_string(r) << std::endl;
        return 1;
    }

    r = builder.buildEngine(param.model_file_path, param.engine_file_path, precision);
    if(r != yolov5::RESULT_SUCCESS) {
        std::cout << "buildEngine() failed: " << yolov5::result_to_string(r) << std::endl;
        return 1;
    }

    return 0;
}