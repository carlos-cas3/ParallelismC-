#include "face_processor.hpp"
#include <iostream>

FaceProcessor::FaceProcessor(const std::string& modelPath,
                               const std::string& supabaseUrl,
                               const std::string& apiKey)
    : arcface(modelPath)
    , http(supabaseUrl)
    , persons(http, apiKey)
    , embeddings(http, apiKey)
{}

bool FaceProcessor::validateFace(const cv::Mat& face) {
    return FaceQualityAssessor::isValid(face);
}

FaceResult FaceProcessor::processRegister(const cv::Mat& face, int faceId, const std::string& personName) {
    FaceResult result;
    result.task_id = 0;
    result.face_id = faceId;
    result.success = false;

    if (face.empty()) {
        std::cerr << "[FaceProcessor] Imagen vacia\n";
        return result;
    }

    if (!validateFace(face)) {
        std::cerr << "[FaceProcessor] Calidad insuficiente\n";
        return result;
    }

    if (personName.empty()) {
        std::cerr << "[FaceProcessor] Nombre vacio\n";
        return result;
    }

    auto embedding = arcface.getEmbedding(face);
    if (embedding.empty()) {
        std::cerr << "[FaceProcessor] Error generando embedding\n";
        return result;
    }

    std::string person_id = persons.getOrCreate(personName);
    if (person_id.empty()) {
        std::cerr << "[FaceProcessor] Error al obtener/crear persona\n";
        return result;
    }

    if (embeddings.save(person_id, embedding)) {
        result.success = true;
        result.person_id = person_id;
        result.person_name = personName;
        result.status = "registered";
        std::cout << "[FaceProcessor] Registrado: " << personName << "\n";
    }

    return result;
}

FaceResult FaceProcessor::processRecognize(const cv::Mat& face, int faceId) {
    FaceResult result;
    result.task_id = 0;
    result.face_id = faceId;
    result.success = false;

    if (face.empty()) {
        std::cerr << "[FaceProcessor] Imagen vacia\n";
        result.person_name = "Desconocido";
        result.confidence = 0.0f;
        return result;
    }

    if (!validateFace(face)) {
        std::cerr << "[FaceProcessor] Calidad insuficiente\n";
        result.person_name = "Desconocido";
        result.confidence = 0.0f;
        return result;
    }

    auto embedding = arcface.getEmbedding(face);
    if (embedding.empty()) {
        std::cerr << "[FaceProcessor] Error generando embedding\n";
        result.person_name = "Desconocido";
        result.confidence = 0.0f;
        return result;
    }

    MatchResult match = embeddings.findBestMatch(embedding, 0.70f);

    if (match.score >= 0.70f) {
        result.success = true;
        result.person_id = match.person_id;
        result.person_name = match.name;
        result.confidence = match.score;
        std::cout << "[FaceProcessor] Reconocido: " << match.name 
                  << " (score=" << match.score << ")\n";
    } else {
        result.person_name = "Desconocido";
        result.confidence = 0.0f;
        std::cout << "[FaceProcessor] No reconocido\n";
    }

    return result;
}
