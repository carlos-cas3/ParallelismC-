#pragma once
#include <opencv2/opencv.hpp>

struct FaceQuality {
    float sharpness;
    float brightness;
    float contrast;
    bool valid;
};

namespace FaceQualityAssessor {
    FaceQuality assess(const cv::Mat& face);
    bool isValid(const cv::Mat& face);
}
