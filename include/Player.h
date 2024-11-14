#pragma once
#include <string>
#include <utility>
#include "message.h"

class Player {
public:
    int socket;
    std::string name;
    std::pair<int, int> position;
    std::string color;
    std::string id;

    explicit Player(int sock);
    void send_message(const std::string& message);
    void send_message(const Message& message);
}; 