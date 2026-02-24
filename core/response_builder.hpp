#pragma once
#include <string>

class ResponseBuilder {
public:
    static std::string buildRegisterSuccess(int faceId, const std::string& personId, const std::string& name) {
        return "{\"face_id\":" + std::to_string(faceId) +
               ",\"person_id\":\"" + personId + "\"" +
               ",\"person_name\":\"" + name + "\"" +
               ",\"status\":\"registered\"}";
    }

    static std::string buildRegisterError(int faceId) {
        return "{\"face_id\":" + std::to_string(faceId) +
               ",\"status\":\"error\"}";
    }

    static std::string buildRecognizeSuccess(int faceId, const std::string& personId, 
                                              const std::string& name, float confidence) {
        std::string json = "{"
            "\"face_id\":" + std::to_string(faceId) + ","
            "\"person_id\":\"" + personId + "\","
            "\"person_name\":\"" + name + "\","
            "\"confidence\":" + std::to_string(confidence) +
            "}";
        return json;
    }

    static std::string buildRecognizeUnknown(int faceId) {
        std::string json = "{"
            "\"face_id\":" + std::to_string(faceId) + ","
            "\"person_id\":\"\","
            "\"person_name\":\"Desconocido\","
            "\"confidence\":0.0"
            "}";
        return json;
    }
};
