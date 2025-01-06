#include "message.h"
#include "player.h"
#include "quoridor_game.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <optional>
#include <map>

Message::Message() {
    type = MessageType::WRONG_MESSAGE;
}

Message::Message(const std::string& message_string) {
    std::istringstream stream(message_string);
    std::string type_str;
    std::string data_str;
    std::getline(stream, type_str, '|');
    type = string_to_message_type(type_str.substr(5)); // Remove "type:"
    data_str = message_string.substr(type_str.length() + 1); // Remove "type:" and "|"
    if (data_str.substr(0, 5) == "data:") {
        extract_data(data_str.substr(5));
    } else {
        type = MessageType::WRONG_MESSAGE;
    }
    
    if (!validate()) {
        type = MessageType::WRONG_MESSAGE;
        set_data("message", "Invalid message structure");
    }
}

bool Message::extract_data(const std::string& data_str) {
    if (data_str.length() == 1 && data_str[0] == ';') {
        return true; // No data, but validly formatted
    }
    std::istringstream stream(data_str);
    std::string pair;
    while (std::getline(stream, pair, ';')) {
        auto delimiter_pos = pair.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = pair.substr(0, delimiter_pos);
            std::string value = pair.substr(delimiter_pos + 1);
            if (value.empty()) {
                return false; // Empty values we do not allow
            }
            data[key] = value;
        }
    }
    return true;
}

void Message::set_type(MessageType msg_type) {
    type = msg_type;
}

void Message::set_data(const std::string& key, const std::string& value) {
    data[key] = value;
}

MessageType Message::get_type() const {
    return type;
}

std::optional<std::string> Message::get_data(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string Message::to_string() const {
    std::string message = "type:" + message_type_to_string(type) + "|data:";
    for (const auto& pair : data) {
        message += pair.first + "=" + pair.second + ";";
    }
    if (data.empty()) {
        message += ";";
    }
    return message;
}

bool Message::validate() const {
    return (type != MessageType::WRONG_MESSAGE);
}

Message Message::create_welcome(const std::string& message) {
    Message msg;
    msg.set_type(MessageType::WELCOME);
    msg.set_data("message", message);
    return msg;
}

Message Message::create_waiting() {
    Message msg;
    msg.set_type(MessageType::WAITING);
    return msg;
}

Message Message::create_game_started(QuoridorGame* game) {
    Message msg = create_next_turn(game);
    msg.set_type(MessageType::GAME_STARTED);
    return msg;
}

Message Message::create_game_ended(QuoridorGame* game, Player* player) {
    Message msg;
    msg.set_type(MessageType::GAME_ENDED);
    msg.set_data("lobby_id", std::to_string(game->get_lobby_id()));
    msg.set_data("winner_id", player->id);
    msg.set_data("board", game->get_board_string());
    return msg;
}

Message Message::create_error(const std::string& message) {
    Message msg;
    msg.set_type(MessageType::ERROR);
    msg.set_data("message", message);
    return msg;
}

Message Message::create_next_turn(QuoridorGame* game) {
    Message msg;
    msg.set_type(MessageType::NEXT_TURN);
    msg.set_data("lobby_id", std::to_string(game->get_lobby_id()));
    msg.set_data("board", game->get_board_string());
    msg.set_data("current_player_id", game->get_players()[game->get_current_player()]->id);

    // send walls
    msg.add_walls(game->get_horizontal_walls(), true);
    msg.add_walls(game->get_vertical_walls(), false);
    
    // Add players using the new method
    msg.add_players(game->get_players());
    
    
    return msg;
}

void Message::add_walls(const std::vector<std::pair<int, int>>& walls, bool is_horizontal) {
    std::string key = (is_horizontal) ? "horizontal_walls" : "vertical_walls";
    std::string value = "";
    for (int i = 0; i < walls.size(); i++) {
        value += "[" + std::to_string(walls[i].first) + "," + std::to_string(walls[i].second) + "]";
        if (i != walls.size() - 1) {
            value += ",";
        }
    }
    if (walls.empty()) {
        value += "[]";
    }
    data[key] = value;
}

void Message::add_players(std::vector<Player*> players) {
    std::string key = "players";
    std::string value = "";
    for (int i = 0; i < players.size(); i++) {
        value += "[id:" + players[i]->id + ",row:" + std::to_string(players[i]->position.first) + ",col:" + std::to_string(players[i]->position.second) + ",name:" + players[i]->name + ",board_char:" + std::string(1, players[i]->get_board_char()) + ",walls_left:" + std::to_string(players[i]->get_walls_left()) + "]";
        if (i != players.size() - 1) {
            value += ",";
        }
    }
    value += ";";
    // here we do not check if the value is empty or not if it is I am throwing the pc out of the window.
    data[key] = value;
}

Message Message::create_name_request() {
    Message msg;
    msg.set_type(MessageType::NAME_REQUEST);
    return msg;
}

Message Message::create_heartbeat() {
    Message msg;
    msg.set_type(MessageType::HEARTBEAT);
    return msg;
}

Message Message::create_ack() {
    Message msg;
    msg.set_type(MessageType::ACK);
    return msg;
}

Message Message::create_player_disconnected(Player* player) {
    Message msg;
    msg.set_type(MessageType::PLAYER_DISCONNECTED);
    msg.set_data("disconnected_player_id", player->id);
    return msg;
}

Message Message::create_player_reconnected(Player* player) {
    Message msg;
    msg.set_type(MessageType::PLAYER_RECONNECTED);
    msg.set_data("reconnected_player_id", player->id);
    return msg;
}

std::string Message::message_type_to_string(MessageType type) {
    switch (type) {
        case MessageType::WELCOME: return "welcome";
        case MessageType::WAITING: return "waiting";
        case MessageType::GAME_STARTED: return "game_started";
        case MessageType::GAME_ENDED: return "game_ended";
        case MessageType::ERROR: return "error";
        case MessageType::WRONG_MESSAGE: return "wrong_message";
        case MessageType::MOVE: return "move";
        case MessageType::ACK: return "ack";
        case MessageType::NEXT_TURN: return "next_turn";
        case MessageType::NAME_REQUEST: return "name_request";
        case MessageType::NAME_RESPONSE: return "name_response";
        case MessageType::HEARTBEAT: return "heartbeat";
        case MessageType::PLAYER_DISCONNECTED: return "player_disconnected";
        case MessageType::PLAYER_RECONNECTED: return "player_reconnected";
        case MessageType::ABANDON: return "abandon";
        default: return "unknown";
    }
}

MessageType Message::string_to_message_type(const std::string& typeStr) {
    if (typeStr == "welcome") return MessageType::WELCOME;
    if (typeStr == "waiting") return MessageType::WAITING;
    if (typeStr == "game_started") return MessageType::GAME_STARTED;
    if (typeStr == "game_ended") return MessageType::GAME_ENDED;
    if (typeStr == "move") return MessageType::MOVE;
    if (typeStr == "ack") return MessageType::ACK;
    if (typeStr == "error") return MessageType::ERROR;
    if (typeStr == "next_turn") return MessageType::NEXT_TURN;
    if (typeStr == "name_request") return MessageType::NAME_REQUEST;
    if (typeStr == "name_response") return MessageType::NAME_RESPONSE;
    if (typeStr == "heartbeat") return MessageType::HEARTBEAT;
    if (typeStr == "player_disconnected") return MessageType::PLAYER_DISCONNECTED;
    if (typeStr == "player_reconnected") return MessageType::PLAYER_RECONNECTED;
    if (typeStr == "abandon") return MessageType::ABANDON;
    return MessageType::WRONG_MESSAGE;
}

