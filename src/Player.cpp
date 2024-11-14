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

void Player::set_id(std::string id) {
    this->id = id;
}

void Player::set_name(std::string name) {
    this->name = name;
}

void Player::set_position(std::pair<int, int> position) {
    this->position = position;
}

void Player::set_color(std::string color) {
    this->color = color;
}