#pragma once
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>    // ← SSL real, no solo SHA/HMAC
#include <openssl/err.h>

class SupabaseClient {
public:
    SupabaseClient(const std::string& project_url,
                   const std::string& api_key);
    ~SupabaseClient();

    std::string getOrCreatePerson(const std::string& name);
    bool saveEmbedding(const std::string& person_id,
                       const std::vector<float>& embedding);
    std::string findBestMatch(const std::vector<float>& embedding,
                              float& best_score,
                              float threshold = 0.60f);

private:
    std::string project_url;
    std::string api_key;
    std::string host;       // ← guardar host parseado una sola vez

    std::string vectorToArrayString(const std::vector<float>& vec);
    std::string escapeJson(const std::string& s);

    std::string httpsRequest(const std::string& method,
                             const std::string& path,
                             const std::string& body = "");

    bool parseJsonField(const std::string& json,
                        const std::string& field,
                        std::string& result);
    bool parseJsonArray(const std::string& json,
                        std::vector<std::string>& results);
};
