#include "receiver.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
ZMQReceiver::ZMQReceiver(const std::string& address)
    : context(1),
      socket(context, zmq::socket_type::pull) {
    socket.bind(address);
    std::cout << "[ZMQ] Listening on " << address << std::endl;
}

bool ZMQReceiver::receive(
    cv::Mat& face,
    int& faceId,
    std::string& mode,
    std::string& personName
) {
    zmq::message_t message;
    
    auto result = socket.recv(message, zmq::recv_flags::none);
    if (!result || message.size() < 13) {
        std::cerr << "[ZMQ] Invalid message size: " << message.size() << "\n";
        return false;
    }

    const uint8_t* data = static_cast<const uint8_t*>(message.data());
    size_t offset = 0;

    uint8_t mode_byte = data[offset++];
    mode = (mode_byte == 1) ? "register" : "recognize";

    uint32_t face_id_net;
    std::memcpy(&face_id_net, data + offset, 4);
    faceId = ntohl(face_id_net);
    offset += 4;

    uint32_t name_len_net;
    std::memcpy(&name_len_net, data + offset, 4);
    uint32_t name_len = ntohl(name_len_net);
    offset += 4;

    personName = "";
    if (name_len > 0 && offset + name_len <= message.size()) {
        personName = std::string(reinterpret_cast<const char*>(data + offset), name_len);
        offset += name_len;
    }

    if (offset + 4 > message.size()) {
        std::cerr << "[ZMQ] Invalid message: no image size\n";
        return false;
    }

    uint32_t img_size_net;
    std::memcpy(&img_size_net, data + offset, 4);
    uint32_t img_size = ntohl(img_size_net);
    offset += 4;

    if (message.size() != offset + img_size) {
        std::cerr << "[ZMQ] Size mismatch. Expected: " << (offset + img_size)
                  << ", Got: " << message.size() << std::endl;
        return false;
    }

    std::vector<uint8_t> img_data(data + offset, data + message.size());
    face = cv::imdecode(img_data, cv::IMREAD_COLOR);

    if (face.empty()) {
        std::cerr << "[ZMQ] Failed to decode image\n";
        return false;
    }

    std::cout << "[ZMQ] Received: mode=" << mode 
              << ", faceId=" << faceId 
              << ", name=" << personName
              << ", size=" << face.size() << std::endl;

    return true;
}
