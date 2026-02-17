#include "arcface.hpp"
#include <numeric>
#include <cmath>

ArcFace::ArcFace(const std::string& model_path) {
    net = cv::dnn::readNetFromONNX(model_path);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

static void l2_normalize(std::vector<float>& v) {
    float norm = std::sqrt(std::inner_product(v.begin(), v.end(), v.begin(), 0.0f));
    for (auto& x : v) x /= norm;
}

std::vector<float> ArcFace::getEmbedding(const cv::Mat& face) {
    cv::Mat resized, blob;

    cv::resize(face, resized, cv::Size(112, 112));

    blob = cv::dnn::blobFromImage(
        resized,
        1.0 / 128.0,
        cv::Size(112, 112),
        cv::Scalar(127.5, 127.5, 127.5),
        false, false
    );

    net.setInput(blob);
    cv::Mat out = net.forward(); // 1x512

    std::vector<float> embedding(out.ptr<float>(), out.ptr<float>() + 512);
    l2_normalize(embedding);

std::cout << "[ArcFace] Embedding sample: ";
for (int i = 0; i < 5; ++i)
    std::cout << embedding[i] << " ";
std::cout << "... size=" << embedding.size() << std::endl;


    return embedding;
}
