#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>

class ArcFace {
public:
    ArcFace(const std::string& model_path);
    std::vector<float> getEmbedding(const cv::Mat& face);

private:
    cv::dnn::Net net;
};
