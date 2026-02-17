#pragma once
#include <string>
#include <vector>
#include "http_client.hpp"

class EmbeddingRepo {
public:
    EmbeddingRepo(HttpClient& http, const std::string& api_key);

    bool        save(const std::string& person_id,
                     const std::vector<float>& embedding);

    std::string findBestMatch(const std::vector<float>& embedding,
                              float& best_score,
                              float threshold = 0.60f);

private:
    HttpClient& http;
    std::string api_key;
};
