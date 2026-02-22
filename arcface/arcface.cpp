#include "arcface.hpp"
#include <numeric>
#include <cmath>
#include <iostream>

ArcFace::ArcFace(const std::string& model_path) {
    net = cv::dnn::readNetFromONNX(model_path);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

float ArcFace::l2_normalize_safe(std::vector<float>& v) {
    float norm = std::sqrt(std::inner_product(v.begin(), v.end(), v.begin(), 0.0f));
    if (norm > 1e-6f) {
        for (auto& x : v) x /= norm;
    }
    return norm;
}

std::vector<float> ArcFace::getEmbedding(const cv::Mat& face) {
    if (face.empty()) {
        std::cerr << "[ArcFace] ERROR: Imagen vacia\n";
        return {};
    }

    if (face.cols < 50 || face.rows < 50) {
        std::cerr << "[ArcFace] ERROR: Imagen muy pequena (" 
                  << face.cols << "x" << face.rows << ")\n";
        return {};
    }

    cv::Mat input = face.clone();
    
    if (input.channels() == 1) {
        cv::cvtColor(input, input, cv::COLOR_GRAY2BGR);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, input, cv::COLOR_BGRA2BGR);
    }

    cv::Mat resized;
    cv::resize(input, resized, cv::Size(112, 112));

    cv::Mat blob = cv::dnn::blobFromImage(
        resized,
        1.0 / 128.0,
        cv::Size(112, 112),
        cv::Scalar(127.5, 127.5, 127.5),
        true,
        false
    );

    net.setInput(blob);
    cv::Mat out = net.forward();

    int dim = out.total();
    if (dim == 0) {
        std::cerr << "[ArcFace] ERROR: Output vacio del modelo\n";
        return {};
    }

    std::vector<float> embedding(out.ptr<float>(), out.ptr<float>() + dim);

    float norm = l2_normalize_safe(embedding);
    if (norm < 1e-6f) {
        std::cerr << "[ArcFace] ERROR: Embedding con norma cero\n";
        return {};
    }

    std::cout << "[ArcFace] OK: dim=" << dim << " norm=" << norm << "\n";

    return embedding;
}
