#pragma once
#include <string>

class Player {
public:
    int socket;
    std::string name;
    std::pair<int, int> position;

    explicit Player(int sock);
    void sendMessage(const std::string& message);
}; 