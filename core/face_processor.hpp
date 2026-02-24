#pragma once
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include "arcface/arcface.hpp"
#include "supabase/http_client.hpp"
#include "supabase/person_repo.hpp"
#include "supabase/embedding_repo.hpp"
#include "supabase/face_quality.hpp"
#include "mpi/task.hpp"

class FaceProcessor {
public:
    FaceProcessor(const std::string& modelPath, 
                  const std::string& supabaseUrl, 
                  const std::string& apiKey);

    FaceResult processRegister(const cv::Mat& face, int faceId, const std::string& personName);
    FaceResult processRecognize(const cv::Mat& face, int faceId);

private:
    ArcFace arcface;
    HttpClient http;
    PersonRepo persons;
    EmbeddingRepo embeddings;

    bool validateFace(const cv::Mat& face);
};
