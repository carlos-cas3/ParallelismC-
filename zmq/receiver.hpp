#pragma once
#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <string>

class ZMQReceiver {
public:
    explicit ZMQReceiver(const std::string& address = "tcp://*:5555");

    // receive con timeout (no bloquea infinitamente)
    // timeout en milisegundos, -1 para bloque infinito (comportamiento original)
    bool receive(
        cv::Mat&     face,
        int&         faceId,
        std::string& mode,
        std::string& personName,
        int          timeoutMs = 100
    );

private:
    zmq::context_t context;
    zmq::socket_t  socket;
    bool           connected = false;

    std::string parseJsonField(const std::string& json, const std::string& key);
};
