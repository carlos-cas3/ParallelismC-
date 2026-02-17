#include "embedding_repo.hpp"
#include "json_utils.hpp"
#include <iostream>

EmbeddingRepo::EmbeddingRepo(HttpClient& http, const std::string& key)
    : http(http), api_key(key) {}

bool EmbeddingRepo::save(
    const std::string& person_id,
    const std::vector<float>& embedding
) {
    std::string body = "{\"person_id\":\"" + person_id +
                       "\",\"embedding\":" +
                       JsonUtils::floatVectorToArray(embedding) + "}";

    std::string response = http.request("POST", "/rest/v1/embeddings", api_key, body);
    std::cout << "[EmbeddingRepo] Save: " << response.substr(0, 100) << "\n";

    return response.find("\"id\"") != std::string::npos;
}

MatchResult EmbeddingRepo::findBestMatch(
    const std::vector<float>& embedding,
    float threshold
) {
    MatchResult result{"", "DESCONOCIDO", 0.0f};

    std::string body =
        "{\"query_embedding\":" + JsonUtils::floatVectorToArray(embedding) +
        ",\"match_threshold\":"  + std::to_string(threshold) +
        ",\"match_count\":5}";

    std::string response = http.request(
        "POST", "/rest/v1/rpc/match_person", api_key, body
    );

    std::vector<std::string> results;
    if (!JsonUtils::parseArray(response, results))
        return result;

    float best_score = -1.0f;

    for (const auto& r : results) {
        std::string person_id, name, sim_str;
        if (JsonUtils::parseField(r, "person_id", person_id) &&
            JsonUtils::parseField(r, "name", name) &&
            JsonUtils::parseField(r, "similarity", sim_str)) {
            float score = std::stof(sim_str);
            if (score > best_score) {
                best_score = score;
                result.person_id = person_id;
                result.name = name;
                result.score = score;
            }
        }
    }

    if (result.score < threshold) {
        result.person_id = "";
        result.name = "DESCONOCIDO";
        result.score = 0.0f;
    }

    return result;
}
