#pragma once
#include <zmq.hpp>
#include <string>

class ZMQSender {
public:
    explicit ZMQSender(const std::string& address = "tcp://*:5556");
    bool send(const std::string& json_response);
private:
    zmq::context_t context;
    zmq::socket_t  socket;
    bool connected = false;
};
