#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <vector>
#include <iostream>
#include "cxxopts.hpp"

class cmdline_params
{
public:
    int frame_width = 0;
    int frame_height = 0;
	int chessboard_width = 7; // Inner corners
    int chessboard_height = 7; // Inner corners
    float square_size = 0.02f; // Size of a square in meters
	int num_frames = 30; // Number of frames to capture
};

void print_help(const cxxopts::Options& options)
{
    std::cout << options.help() << std::endl;
    std::cout << std::endl << "For camera calibration, you need to specify the frame width and height, "
        "the number of inner corners in the chessboard pattern (both horizontal and vertical), "
        "the size of one square in meters, and optionally the number of frames to capture." << std::endl;
    std::cout << "Example: camera_calibration -w 640 -h 480 -a 7 -b 7 -s 0.02 -n 30" << std::endl;

	exit(0);
}

void parse_command_line(int argc, char* argv[], cmdline_params* param)
{
    if (param == nullptr)
        return;

    cxxopts::Options options("", "");
    options.allow_unrecognised_options();
    options.add_options()
        ("h,help", "show this help menu")
        ("w,frame_width", "[mandatory] specify the frame width", cxxopts::value<int>())
        ("h,frame_height", "[mandatory] specify the frame height", cxxopts::value<int>())
        ("a,chessboard_width", "[mandatory] specify the vertical number of inner corners", cxxopts::value<int>())
        ("b,chessboard_height", "[mandatory] specify the horizontal number of inner corners", cxxopts::value<int>())
        ("s,square_size", "[mandatory] specify size in meters of one square", cxxopts::value<double>())
        ("n,num_frames", "[optional] ", cxxopts::value<int>());

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        print_help(options);
    }

    if (result.count("frame_width")) {
        param->frame_width = result["frame_width"].as<int>();
    }
    else {
        print_help(options);
    }

    if (result.count("frame_height")) {
        param->frame_height = result["frame_height"].as<int>();
    }
    else {
        print_help(options);
    }

    if (result.count("chessboard_width")) {
        param->chessboard_width = result["chessboard_width"].as<int>();
    }
    else {
        print_help(options);
    }

    if (result.count("chessboard_height")) {
        param->frame_height = result["chessboard_height"].as<int>();
    }
    else {
        print_help(options);
    }

    if (result.count("square_size")) {
        param->frame_height = result["square_size"].as<double>();
    }
    else {
        print_help(options);
    }

    if (result.count("num_frames")) {
        param->num_frames = result["num_frames"].as<int>();
    }
}

int main(int argc, char* argv[]) {
	cmdline_params params;
	parse_command_line(argc, argv, &params);

    const int CHECKERBOARD[2] = { params.chessboard_width, params.chessboard_height };
    const float squareSize = params.square_size;   

    // Prepare object points (3D points in the real world)
    std::vector<cv::Point3f> objp;
    for (int i = 0; i < CHECKERBOARD[1]; i++) {
        for (int j = 0; j < CHECKERBOARD[0]; j++) {
            objp.emplace_back(j * squareSize, i * squareSize, 0);
        }
    }

    // Storage for object points and image points
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<std::vector<cv::Point2f>> imagePoints;

    cv::Size imageSize(params.frame_width, params.frame_height);

    auto capture = new cv::VideoCapture(1, cv::CAP_DSHOW);
    capture->set(cv::CAP_PROP_FRAME_WIDTH, params.frame_width);
    capture->set(cv::CAP_PROP_FRAME_HEIGHT, params.frame_height);

    bool completed = false;
    int num = 0;

    while (!completed) {
        cv::Mat image;
        *capture >> image;
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        cv::threshold(image, image, 40, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        if (image.empty()) {
            std::cerr << "Failed to load image: " << std::endl;
            continue;
        }

        std::vector<cv::Point2f> corners;

        bool found = cv::findChessboardCorners(image, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corners, cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_ADAPTIVE_THRESH); // 
        cv::drawChessboardCorners(image, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corners, found);
        if (found) {
            cv::cornerSubPix(image, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.001));
            imagePoints.push_back(corners);
            objectPoints.push_back(objp);

			std::cout << "Found corners in image." << std::endl;
        }

        cv::imshow("Corners", image);
        cv::waitKey(500);

        num++;
        if (num >= params.num_frames) { 
            completed = true;
		}
    }
    cv::destroyAllWindows();

    // Fisheye calibration
    cv::Mat K = cv::Mat::eye(3, 3, CV_64F); // Intrinsic matrix
    cv::Mat D = cv::Mat::zeros(4, 1, CV_64F); // Distortion coefficients
    std::vector<cv::Mat> rvecs, tvecs;

    if (!objectPoints.empty() && !imagePoints.empty()) {
        int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC |
            cv::fisheye::CALIB_CHECK_COND |
            cv::fisheye::CALIB_FIX_SKEW;

        double rms = cv::fisheye::calibrate(objectPoints, imagePoints, imageSize, K, D, rvecs, tvecs, flags,
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 1e-6));

        std::cout << "Calibration RMS error: " << rms << std::endl;
        std::cout << "Intrinsic Matrix (K):\n" << K << std::endl;
        std::cout << "Distortion Coefficients (D):\n" << D << std::endl;
    }
    else {
        std::cerr << "Not enough valid data for calibration!" << std::endl;
    }

    return 0;
}
