#pragma once
#include <string>
#include "http_client.hpp"

class PersonRepo {
public:
    PersonRepo(HttpClient& http, const std::string& api_key);

    // Retorna el UUID de la persona (crea si no existe)
    std::string getOrCreate(const std::string& name);

private:
    HttpClient&  http;
    std::string  api_key;
};
