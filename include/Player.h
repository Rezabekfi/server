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

    void set_id(std::string id);
    void set_name(std::string name);
    void set_position(std::pair<int, int> position);
    void set_color(std::string color);

    void send_message(const std::string& message);
    void send_message(const Message& message);
}; 