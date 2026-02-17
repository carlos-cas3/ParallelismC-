#include "receiver.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

ZMQReceiver::ZMQReceiver(const std::string& address)
    : context(1), socket(context, zmq::socket_type::pull) {
    try {
        socket.bind(address);
        connected = true;
        std::cout << "[ZMQ] Listening on " << address << "\n";
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQ] Bind failed: " << e.what() << "\n";
        connected = false;
    }
}

// Parser JSON minimalista para extraer strings y números
std::string ZMQReceiver::parseJsonField(
    const std::string& json,
    const std::string& key
) {
    // Buscar "key":"value" o "key":value
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos += search.size();

    if (json[pos] == '"') {
        // Valor string
        pos++;
        size_t end = json.find('"', pos);
        return (end != std::string::npos) ? json.substr(pos, end - pos) : "";
    } else {
        // Valor numérico
        size_t end = json.find_first_of(",}", pos);
        return (end != std::string::npos) ? json.substr(pos, end - pos) : "";
    }
}

bool ZMQReceiver::receive(
    cv::Mat&     face,
    int&         faceId,
    std::string& mode,
    std::string& personName
) {
    if (!connected) {
        std::cerr << "[ZMQ] Socket not connected\n";
        return false;
    }

    zmq::message_t message;
    auto result = socket.recv(message, zmq::recv_flags::none);
    if (!result || message.size() < 5) {
        std::cerr << "[ZMQ] Mensaje inválido\n";
        return false;
    }

    const uint8_t* data = static_cast<const uint8_t*>(message.data());

    // Leer tamaño del header JSON (4 bytes big-endian)
    uint32_t header_len_net;
    std::memcpy(&header_len_net, data, 4);
    uint32_t header_len = ntohl(header_len_net);

    if (header_len == 0 || header_len > message.size() - 4) {
        std::cerr << "[ZMQ] Header size inválido: " << header_len << "\n";
        return false;
    }

    // Extraer JSON header
    std::string header_json(
        reinterpret_cast<const char*>(data + 4), header_len
    );

    // Extraer imagen (resto del mensaje)
    size_t img_offset = 4 + header_len;
    size_t img_size   = message.size() - img_offset;

    if (img_size == 0) {
        std::cerr << "[ZMQ] Sin imagen en el mensaje\n";
        return false;
    }

    // Parsear campos del JSON
    mode       = parseJsonField(header_json, "mode");
    personName = parseJsonField(header_json, "person_name");

    // face_id puede venir como face_id o face_track_id
    std::string id_str = parseJsonField(header_json, "face_track_id");
    if (id_str.empty())
        id_str = parseJsonField(header_json, "face_id");
    faceId = id_str.empty() ? 0 : std::stoi(id_str);

    if (mode.empty()) {
        std::cerr << "[ZMQ] Mode no encontrado en header\n";
        return false;
    }

    // Decodificar imagen
    std::vector<uint8_t> img_data(
        data + img_offset,
        data + message.size()
    );
    face = cv::imdecode(img_data, cv::IMREAD_COLOR);

    if (face.empty()) {
        std::cerr << "[ZMQ] Error decodificando imagen\n";
        return false;
    }

    std::cout << "[ZMQ] OK: mode=" << mode
              << " faceId=" << faceId
              << " name='" << personName << "'"
              << " img=" << face.cols << "x" << face.rows << "\n";

    return true;
}
