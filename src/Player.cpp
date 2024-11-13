#include "player.h"
#include <sys/socket.h>

Player::Player(int sock) : socket(sock) {}

void Player::send_message(const std::string& message) {
    send(socket, message.c_str(), message.length(), 0);
} 
