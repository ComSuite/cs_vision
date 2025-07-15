#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <vector>
#include <iostream>

int main() {
    // Chessboard dimensions
    const int CHECKERBOARD[2] = { 7, 7 }; // Width x Height (inner corners)
    const float squareSize = 0.02f;   // Size of a square in meters (adjust as needed)

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

    // Load calibration images
    //std::vector<std::string> imageFiles = { "image1.jpg", "image2.jpg", "image3.jpg" }; // Add your image paths
    cv::Size imageSize(1920, 1080);

    auto capture = new cv::VideoCapture(1, cv::CAP_DSHOW);
    capture->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    capture->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    bool completed = false;
    int num = 0;

    while (!completed) {
        cv::Mat image; // = cv::imread(file, cv::IMREAD_GRAYSCALE);
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
        if (num >= 30) { 
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
