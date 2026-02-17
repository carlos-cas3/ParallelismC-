#pragma once
#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <string>

class ZMQReceiver {
public:
    ZMQReceiver(const std::string& address = "tcp://*:5555");
    
    // Recibe cara desde Python
    // Retorna true si recibió correctamente
    bool receive(
        cv::Mat& face,
        int& faceId,
        std::string& mode,
        std::string& personName
    );

private:
    zmq::context_t context;
    zmq::socket_t socket;
};
