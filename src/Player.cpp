#include "player.h"
#include <sys/socket.h>
#include <iostream>
#include "message.h"

Player::Player(int sock) : socket(sock) {}

void Player::send_message(const std::string& message) {
    std::cout << "Sending message: " << message << std::endl;
    std::string msg = message + "\n";
    send(socket, msg.c_str(), msg.length()+1, 0);
}

void Player::send_message(const Message& message) {
    send_message(message.to_json());
}