#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>

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
    std::string project_ref;

    std::string extractHostFromUrl(const std::string& url);
    std::string extractPathFromUrl(const std::string& url);
    std::string vectorToArrayString(const std::vector<float>& vec);
    std::string escapeJson(const std::string& s);

    std::string httpRequest(const std::string& method,
                            const std::string& path,
                            const std::string& body = "",
                            const std::string& content_type = "application/json");

    bool parseJsonResponse(const std::string& json,
                          const std::string& field,
                          std::string& result);
    bool parseJsonArrayResponse(const std::string& json,
                                std::vector<std::string>& results);
};

