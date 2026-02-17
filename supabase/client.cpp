#include "client.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>

SupabaseClient::SupabaseClient(const std::string& url, const std::string& key)
    : project_url(url), api_key(key) {
    
    std::regex ref_regex("https://([^.]+)\\.supabase\\.co");
    std::smatch match;
    if (std::regex_search(url, match, ref_regex)) {
        project_ref = match[1].str();
    }
}

SupabaseClient::~SupabaseClient() {
}

std::string SupabaseClient::extractHostFromUrl(const std::string& url) {
    std::regex host_regex("https://([^/]+)");
    std::smatch match;
    if (std::regex_search(url, match, host_regex)) {
        return match[1].str();
    }
    return "";
}

std::string SupabaseClient::extractPathFromUrl(const std::string& url) {
    size_t pos = url.find(".co");
    if (pos != std::string::npos) {
        return url.substr(pos + 3);
    }
    return "/";
}

std::string SupabaseClient::escapeJson(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else result += c;
    }
    return result;
}

std::string SupabaseClient::vectorToArrayString(const std::vector<float>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ",";
        oss << vec[i];
    }
    oss << "]";
    return oss.str();
}

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string SupabaseClient::httpRequest(const std::string& method,
                                        const std::string& path,
                                        const std::string& body,
                                        const std::string& content_type) {
    std::string host = extractHostFromUrl(project_url);
    if (host.empty()) {
        host = project_ref + ".supabase.co";
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[Supabase] Socket creation failed\n";
        return "";
    }
    
    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        std::cerr << "[Supabase] DNS lookup failed for " << host << "\n";
        close(sock);
        return "";
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(443);
    memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[Supabase] Connection failed\n";
        close(sock);
        return "";
    }
    
    std::ostringstream request;
    request << method << " " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "apikey: " << api_key << "\r\n";
    request << "Authorization: Bearer " << api_key << "\r\n";
    request << "Content-Type: " << content_type << "\r\n";
    if (!body.empty()) {
        request << "Content-Length: " << body.size() << "\r\n";
    }
    request << "Connection: close\r\n";
    request << "\r\n";
    if (!body.empty()) {
        request << body;
    }
    
    std::string request_str = request.str();
    send(sock, request_str.c_str(), request_str.size(), 0);
    
    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        response.append(buffer, n);
    }
    
    close(sock);
    
    size_t header_end = response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return response.substr(header_end + 4);
    }
    return response;
}

bool SupabaseClient::parseJsonResponse(const std::string& json,
                                       const std::string& field,
                                       std::string& result) {
    std::string pattern = "\"" + field + "\":\"([^\"]+)\"";
    std::regex re(pattern);
    std::smatch match;
    if (std::regex_search(json, match, re)) {
        result = match[1].str();
        return true;
    }
    
    pattern = "\"" + field + "\":([0-9.]+)";
    re = pattern;
    if (std::regex_search(json, match, re)) {
        result = match[1].str();
        return true;
    }
    
    return false;
}

bool SupabaseClient::parseJsonArrayResponse(const std::string& json,
                                            std::vector<std::string>& results) {
    std::regex re("\\{[^}]+\\}");
    for (std::sregex_iterator it(json.begin(), json.end(), re); it != std::sregex_iterator(); ++it) {
        results.push_back(it->str());
    }
    return !results.empty();
}

std::string SupabaseClient::getOrCreatePerson(const std::string& name) {
    std::cout << "[Supabase] Looking for person: " << name << "\n";
    
    std::string path = "/rest/v1/persons?name=eq." + escapeJson(name) + "&select=id";
    std::string response = httpRequest("GET", path);
    
    if (response.empty()) {
        std::cerr << "[Supabase] Empty response\n";
        return "";
    }
    
    std::vector<std::string> results;
    if (parseJsonArrayResponse(response, results) && !results.empty()) {
        std::string id;
        if (parseJsonResponse(results[0], "id", id)) {
            std::cout << "[Supabase] Found person with id: " << id << "\n";
            return id;
        }
    }
    
    std::cout << "[Supabase] Person not found, creating new...\n";
    
    std::string body = "{\"name\":\"" + escapeJson(name) + "\"}";
    path = "/rest/v1/persons";
    response = httpRequest("POST", path, body);
    
    if (parseJsonResponse(response, "id", body)) {
        std::cout << "[Supabase] Created person with id: " << body << "\n";
        return body;
    }
    
    path = "/rest/v1/persons?name=eq." + escapeJson(name) + "&select=id";
    response = httpRequest("GET", path);
    if (parseJsonArrayResponse(response, results) && !results.empty()) {
        std::string id;
        if (parseJsonResponse(results[0], "id", id)) {
            std::cout << "[Supabase] Retrieved person id: " << id << "\n";
            return id;
        }
    }
    
    return "";
}

bool SupabaseClient::saveEmbedding(const std::string& person_id,
                                   const std::vector<float>& embedding) {
    std::cout << "[Supabase] Saving embedding for person: " << person_id << "\n";
    
    std::string embedding_str = vectorToArrayString(embedding);
    std::string body = "{\"person_id\":\"" + person_id + "\",\"embedding\":" + embedding_str + "}";
    
    std::string path = "/rest/v1/embeddings";
    std::string response = httpRequest("POST", path, body);
    
    std::cout << "[Supabase] Save response: " << response.substr(0, 200) << "\n";
    
    return response.find("201") != std::string::npos || 
           response.find("Created") != std::string::npos ||
           response.find("\"id\"") != std::string::npos;
}

std::string SupabaseClient::findBestMatch(const std::vector<float>& embedding,
                                          float& best_score,
                                          float threshold) {
    std::cout << "[Supabase] Searching for best match...\n";
    
    std::string embedding_str = vectorToArrayString(embedding);
    std::string body = "{\"query_embedding\":" + embedding_str + 
                       ",\"match_threshold\":" + std::to_string(threshold) + 
                       ",\"match_count\":5}";
    
    std::string path = "/rest/v1/rpc/match_person";
    std::string response = httpRequest("POST", path, body);
    
    if (response.empty()) {
        std::cerr << "[Supabase] Empty response from match_person\n";
        return "DESCONOCIDO";
    }
    
    std::cout << "[Supabase] Match response: " << response.substr(0, 300) << "\n";
    
    std::vector<std::string> results;
    if (!parseJsonArrayResponse(response, results) || results.empty()) {
        return "DESCONOCIDO";
    }
    
    best_score = -1.0f;
    std::string best_name = "DESCONOCIDO";
    
    for (const auto& result : results) {
        std::string name, similarity_str;
        float similarity = 0.0f;
        
        if (parseJsonResponse(result, "name", name) && 
            parseJsonResponse(result, "similarity", similarity_str)) {
            similarity = std::stof(similarity_str);
            
            float score = 1.0f - similarity;
            
            std::cout << "[Supabase] Match: " << name << " (score=" << score << ")\n";
            
            if (score > best_score) {
                best_score = score;
                best_name = name;
            }
        }
    }
    
    if (best_score < threshold) {
        return "DESCONOCIDO";
    }
    
    return best_name;
}

