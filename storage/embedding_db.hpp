#pragma once
#include <string>
#include <vector>

class EmbeddingDB {
public:
    EmbeddingDB(const std::string& basePath = "storage/embeddings");

    // -------- REGISTRO --------
    void save(const std::string& person,
              const std::vector<float>& embedding);

    // -------- RECONOCIMIENTO --------
    void load();
    std::string recognize(const std::vector<float>& emb,
                           float& bestScore,
                           float threshold = 0.60f);

private:
    std::string basePath;

    // Solo para reconocimiento (RAM)
    std::vector<std::string> names;
    std::vector<std::vector<float>> embeddings;

    float cosineSimilarity(const std::vector<float>& a,
                           const std::vector<float>& b);
};

