#include "embedding_db.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cmath>

namespace fs = std::filesystem;

EmbeddingDB::EmbeddingDB(const std::string& path)
    : basePath(path) {
    fs::create_directories(basePath);
}

/* ================= REGISTRO ================= */

void EmbeddingDB::save(const std::string& person,
                       const std::vector<float>& emb) {

    fs::path personDir = fs::path(basePath) / person;
    fs::create_directories(personDir);

    int count = std::distance(
        fs::directory_iterator(personDir),
        fs::directory_iterator{}
    );

    fs::path file =
        personDir / (std::to_string(count + 1) + ".txt");

    std::ofstream f(file);
    for (float v : emb)
        f << v << " ";
    f.close();

    std::cout << "[REGISTER] " << person
              << " → embedding #" << (count + 1) << "\n";
}

/* ================= RECONOCIMIENTO ================= */

void EmbeddingDB::load() {
    names.clear();
    embeddings.clear();

    for (auto& personDir : fs::directory_iterator(basePath)) {
        if (!personDir.is_directory()) continue;

        std::string person = personDir.path().filename().string();

        for (auto& file : fs::directory_iterator(personDir)) {
            if (file.path().extension() != ".txt") continue;

            std::ifstream f(file.path());
            if (!f) continue;

            std::vector<float> emb(512);
            for (float& v : emb)
                f >> v;

            names.push_back(person);
            embeddings.push_back(emb);
        }
    }

    std::cout << "[DB] Loaded " << embeddings.size()
              << " embeddings\n";
}

float EmbeddingDB::cosineSimilarity(
    const std::vector<float>& a,
    const std::vector<float>& b) {

    float dot = 0, na = 0, nb = 0;
    for (int i = 0; i < 512; ++i) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    return dot / (std::sqrt(na) * std::sqrt(nb));
}

std::string EmbeddingDB::recognize(
    const std::vector<float>& emb,
    float& bestScore,
    float threshold) {

    bestScore = -1.0f;
    std::string bestName = "DESCONOCIDO";

    for (size_t i = 0; i < embeddings.size(); ++i) {
        float s = cosineSimilarity(emb, embeddings[i]);
        if (s > bestScore) {
            bestScore = s;
            bestName = names[i];
        }
    }

    if (bestScore < threshold)
        return "DESCONOCIDO";

    return bestName;
}

