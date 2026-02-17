#include "sender.hpp"
#include <iostream>

ZMQSender::ZMQSender(const std::string& address)
    : context(1), socket(context, zmq::socket_type::push) {
    try {
        socket.bind(address);
        connected = true;
        std::cout << "[ZMQSender] Bound to " << address << "\n";
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQSender] Bind failed: " << e.what() << "\n";
        connected = false;
    }
}

bool ZMQSender::send(const std::string& json_response) {
    if (!connected) return false;
    try {
        zmq::message_t msg(json_response.begin(), json_response.end());
        socket.send(msg, zmq::send_flags::none);
        return true;
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZMQSender] Send failed: " << e.what() << "\n";
        return false;
    }
}
