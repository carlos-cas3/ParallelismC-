#include "receiver.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    ZMQReceiver receiver("tcp://*:5555");

    if (!receiver.isConnected()) {
        std::cerr << "No se pudo iniciar el receiver\n";
        return 1;
    }

    while (true) {
        cv::Mat face;
        int faceId;
        std::string mode;
        std::string name;

        if (receiver.receive(face, faceId, mode, name)) {
            cv::imshow("Received Face", face);
            cv::waitKey(1);
        }
    }

    return 0;
}

