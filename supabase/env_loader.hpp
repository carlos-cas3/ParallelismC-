#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>

class EnvLoader {
public:
    static void load(const std::string& path = ".env") {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[ENV] No se encontró: " << path << "\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Ignorar comentarios y líneas vacías
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key   = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            // Setear como variable de entorno
            setenv(key.c_str(), value.c_str(), 1);
        }

        std::cout << "[ENV] Cargado: " << path << "\n";
    }

    static std::string get(const std::string& key, const std::string& fallback = "") {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : fallback;
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\"'");
        size_t end   = s.find_last_not_of(" \t\"'");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
};
