#include "face_quality.hpp"
#include <iostream>

FaceQuality FaceQualityAssessor::assess(const cv::Mat& face) {
    FaceQuality q{0.0f, 0.0f, 0.0f, false};

    if (face.empty()) {
        return q;
    }

    cv::Mat gray;
    if (face.channels() == 3) {
        cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
    } else if (face.channels() == 1) {
        gray = face;
    } else {
        return q;
    }

    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);
    cv::Scalar mean_lap, stddev_lap;
    cv::meanStdDev(laplacian, mean_lap, stddev_lap);
    q.sharpness = std::min(1.0f, static_cast<float>(stddev_lap[0]) / 100.0f);

    cv::Scalar mean_color = cv::mean(face);
    q.brightness = static_cast<float>(mean_color[0]) / 255.0f;

    cv::Scalar mean_gray, stddev_gray;
    cv::meanStdDev(gray, mean_gray, stddev_gray);
    q.contrast = std::min(1.0f, static_cast<float>(stddev_gray[0]) / 80.0f);

    q.valid = (q.sharpness > 0.15f) &&
              (q.brightness > 0.15f) && (q.brightness < 0.95f) &&
              (q.contrast > 0.15f);

    return q;
}

bool FaceQualityAssessor::isValid(const cv::Mat& face) {
    FaceQuality q = assess(face);
    
    if (!q.valid) {
        std::cout << "[FaceQuality] RECHAZADA: sharp=" << q.sharpness
                  << " bright=" << q.brightness
                  << " contrast=" << q.contrast << "\n";
    }
    
    return q.valid;
}
