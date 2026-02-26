#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

enum class TaskType : uint8_t { REGISTER = 0, RECOGNIZE = 1 };

struct FaceTask {
    int task_id;
    int face_id;
    TaskType type;
    std::string person_name;
    std::vector<uint8_t> image_data;
};

struct FaceResult {
    int task_id;
    int face_id;
    bool success;
    std::string person_id;
    std::string person_name;
    float confidence;
    std::string status;
};

inline std::vector<uint8_t> serializeTask(const FaceTask& task) {
    uint32_t name_len = static_cast<uint32_t>(task.person_name.size());
    uint32_t img_len = static_cast<uint32_t>(task.image_data.size());
    
    size_t offset = 0;
    std::vector<uint8_t> buffer(4 + 4 + 1 + 4 + name_len + 4 + img_len);
    
    uint32_t val;
    
    val = htonl(static_cast<uint32_t>(task.task_id));
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    
    val = htonl(static_cast<uint32_t>(task.face_id));
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    
    buffer[offset++] = static_cast<uint8_t>(task.type);
    
    val = htonl(name_len);
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    if (name_len > 0) {
        std::memcpy(&buffer[offset], task.person_name.data(), name_len); offset += name_len;
    }
    
    val = htonl(img_len);
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    if (img_len > 0) {
        std::memcpy(&buffer[offset], task.image_data.data(), img_len);
    }
    
    return buffer;
}

inline FaceTask deserializeTask(const std::vector<uint8_t>& data) {
    FaceTask t;
    size_t offset = 0;
    
    uint32_t val;
    
    std::memcpy(&val, &data[offset], 4); t.task_id = static_cast<int>(ntohl(val)); offset += 4;
    
    std::memcpy(&val, &data[offset], 4); t.face_id = static_cast<int>(ntohl(val)); offset += 4;
    
    t.type = static_cast<TaskType>(data[offset++]);
    
    std::memcpy(&val, &data[offset], 4); uint32_t name_len = ntohl(val); offset += 4;
    if (name_len > 0) {
        t.person_name.assign(reinterpret_cast<const char*>(&data[offset]), name_len);
        offset += name_len;
    }
    
    std::memcpy(&val, &data[offset], 4); uint32_t img_len = ntohl(val); offset += 4;
    if (img_len > 0) {
        t.image_data.assign(&data[offset], &data[offset] + img_len);
    }
    
    return t;
}

inline std::vector<uint8_t> serializeResult(const FaceResult& r) {
    uint32_t pid_len = static_cast<uint32_t>(r.person_id.size());
    uint32_t pn_len = static_cast<uint32_t>(r.person_name.size());
    uint32_t st_len = static_cast<uint32_t>(r.status.size());
    
    size_t offset = 0;
    std::vector<uint8_t> buffer(4 + 4 + 1 + 4 + pid_len + 4 + pn_len + 4 + 4 + st_len);
    
    uint32_t val;
    float conf = r.confidence;
    uint32_t conf_bits;
    std::memcpy(&conf_bits, &conf, 4);
    conf_bits = htonl(conf_bits);
    
    val = htonl(static_cast<uint32_t>(r.task_id));
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    
    val = htonl(static_cast<uint32_t>(r.face_id));
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    
    buffer[offset++] = r.success ? 1 : 0;
    
    val = htonl(pid_len);
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    if (pid_len > 0) {
        std::memcpy(&buffer[offset], r.person_id.data(), pid_len); offset += pid_len;
    }
    
    val = htonl(pn_len);
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    if (pn_len > 0) {
        std::memcpy(&buffer[offset], r.person_name.data(), pn_len); offset += pn_len;
    }
    
    std::memcpy(&buffer[offset], &conf_bits, 4); offset += 4;
    
    val = htonl(st_len);
    std::memcpy(&buffer[offset], &val, 4); offset += 4;
    if (st_len > 0) {
        std::memcpy(&buffer[offset], r.status.data(), st_len);
    }
    
    return buffer;
}

inline FaceResult deserializeResult(const std::vector<uint8_t>& data) {
    FaceResult r;
    size_t offset = 0;
    
    uint32_t val;
    
    std::memcpy(&val, &data[offset], 4); r.task_id = static_cast<int>(ntohl(val)); offset += 4;
    
    std::memcpy(&val, &data[offset], 4); r.face_id = static_cast<int>(ntohl(val)); offset += 4;
    
    r.success = (data[offset++] == 1);
    
    std::memcpy(&val, &data[offset], 4); uint32_t pid_len = ntohl(val); offset += 4;
    if (pid_len > 0) {
        r.person_id.assign(reinterpret_cast<const char*>(&data[offset]), pid_len);
        offset += pid_len;
    }
    
    std::memcpy(&val, &data[offset], 4); uint32_t pn_len = ntohl(val); offset += 4;
    if (pn_len > 0) {
        r.person_name.assign(reinterpret_cast<const char*>(&data[offset]), pn_len);
        offset += pn_len;
    }
    
    uint32_t conf_bits;
    std::memcpy(&conf_bits, &data[offset], 4); conf_bits = ntohl(conf_bits);
    std::memcpy(&r.confidence, &conf_bits, 4); offset += 4;
    
    std::memcpy(&val, &data[offset], 4); uint32_t st_len = ntohl(val); offset += 4;
    if (st_len > 0) {
        r.status.assign(reinterpret_cast<const char*>(&data[offset]), st_len);
    }
    
    return r;
}
