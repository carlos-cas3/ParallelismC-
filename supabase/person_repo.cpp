#include "person_repo.hpp"
#include "json_utils.hpp"
#include <iostream>
#include <vector>

PersonRepo::PersonRepo(HttpClient& http, const std::string& key)
    : http(http), api_key(key) {}

std::string PersonRepo::getOrCreate(const std::string& name) {
    // Buscar existente
    std::string path = "/rest/v1/persons?name=eq." +
                       JsonUtils::escapeString(name) + "&select=id";
    std::string response = http.request("GET", path, api_key);

    std::vector<std::string> items;
    if (JsonUtils::parseArray(response, items)) {
        std::string id;
        if (JsonUtils::parseField(items[0], "id", id)) {
            std::cout << "[PersonRepo] Found: " << id << "\n";
            return id;
        }
    }

    // Crear nuevo
    std::string body = "{\"name\":\"" + JsonUtils::escapeString(name) + "\"}";
    response = http.request("POST", "/rest/v1/persons", api_key, body);
    std::cout << "[PersonRepo] Raw response: " << response << "\n";

    // La respuesta es un array: [{"id":"...","name":"..."}]
    // Hay que parsear el primer elemento
    std::string id;
    items.clear();
    if (JsonUtils::parseArray(response, items) && !items.empty()) {
        if (JsonUtils::parseField(items[0], "id", id)) {
            std::cout << "[PersonRepo] Created id: " << id << "\n";
            return id;
        }
    }

    return "";
}
