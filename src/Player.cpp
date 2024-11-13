#include "player.h"
#include <sys/socket.h>
#include <iostream>

Player::Player(int sock) : socket(sock) {}

void Player::send_message(const std::string& message) {
    std::string msg = message + "\n";
    send(socket, msg.c_str(), msg.length()+1, 0);
}
