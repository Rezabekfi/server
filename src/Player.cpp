#include "player.h"
#include <sys/socket.h>
#include <iostream>
#include "message.h"

// Define static const members
const int Player::HEARTBEAT_INTERVAL;
const int Player::NORMAL_HEARTBEAT_TIMEOUT;
const int Player::RECONNECTION_HEARTBEAT_TIMEOUT;

Player::Player(int sock) : socket(sock), is_reconnecting(false) {}

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

void Player::set_walls_left(int walls_left) {
    this->walls_left = walls_left;
}

int Player::get_walls_left() const {
    return this->walls_left;
}

void Player::set_goal_row(int goal_row) {
    this->goal_row = goal_row;
}

int Player::get_goal_row() const {
    return this->goal_row;
}

int Player::get_game_id() const {
    return this->game_id;
}

void Player::set_game_id(int game_id) {
    this->game_id = game_id;
}

void Player::update_heartbeat() {
    last_heartbeat = std::chrono::steady_clock::now();
    is_connected = true;
}

bool Player::check_connection() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_heartbeat).count();
    return duration < NORMAL_HEARTBEAT_TIMEOUT;
}

void Player::set_board_char(char board_char) {
    this->board_char = board_char;
}

char Player::get_board_char() const {
    return this->board_char;
}