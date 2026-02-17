#pragma once
#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <string>

class ZMQReceiver {
public:
    explicit ZMQReceiver(const std::string& address = "tcp://*:5555");

    bool receive(
        cv::Mat&     face,
        int&         faceId,
        std::string& mode,
        std::string& personName
    );

private:
    zmq::context_t context;
    zmq::socket_t  socket;
    bool           connected = false;

    std::string parseJsonField(const std::string& json, const std::string& key);
};
