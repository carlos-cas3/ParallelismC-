#pragma once
#include <string>
#include <vector>
#include "http_client.hpp"

struct MatchResult {
    std::string person_id;
    std::string name;
    float score;
};

class EmbeddingRepo {
public:
    EmbeddingRepo(HttpClient& http, const std::string& api_key);

    bool        save(const std::string& person_id,
                     const std::vector<float>& embedding);

    MatchResult findBestMatch(const std::vector<float>& embedding,
                              float threshold = 0.60f);

private:
    HttpClient& http;
    std::string api_key;
};
