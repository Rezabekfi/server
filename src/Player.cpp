#include "player.h"
#include <sys/socket.h>
#include <iostream>

Player::Player(int sock) : socket(sock) {}

void Player::send_message(const std::string& message) {
    std::string msg = message + "\n";
    ssize_t bytes_sent = send(socket, msg.c_str(), msg.length(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Send error: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Sent " << bytes_sent << " bytes: " << msg << std::endl;
    }
}
