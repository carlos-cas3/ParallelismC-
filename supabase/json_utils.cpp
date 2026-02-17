#include "json_utils.hpp"
#include <sstream>
#include <regex>

std::string JsonUtils::escapeString(const std::string& s) {
    std::string r;
    for (char c : s) {
        if      (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else                r += c;
    }
    return r;
}

std::string JsonUtils::floatVectorToArray(const std::vector<float>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ",";
        oss << vec[i];
    }
    oss << "]";
    return oss.str();
}

bool JsonUtils::parseField(
    const std::string& json,
    const std::string& field,
    std::string& result
) {
    std::smatch m;
    std::regex re_str("\"" + field + "\":\"([^\"]+)\"");
    if (std::regex_search(json, m, re_str)) { result = m[1]; return true; }

    std::regex re_num("\"" + field + "\":([0-9.eE+-]+)");
    if (std::regex_search(json, m, re_num)) { result = m[1]; return true; }

    return false;
}

bool JsonUtils::parseArray(
    const std::string& json,
    std::vector<std::string>& results
) {
    std::regex re("\\{[^}]+\\}");
    for (std::sregex_iterator it(json.begin(), json.end(), re);
         it != std::sregex_iterator(); ++it)
        results.push_back(it->str());
    return !results.empty();
}
