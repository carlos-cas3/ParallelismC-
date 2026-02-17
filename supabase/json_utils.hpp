#pragma once
#include <string>
#include <vector>

namespace JsonUtils {
    std::string escapeString(const std::string& s);
    std::string floatVectorToArray(const std::vector<float>& vec);
    bool        parseField(const std::string& json,
                           const std::string& field,
                           std::string& result);
    bool        parseArray(const std::string& json,
                           std::vector<std::string>& results);
}
