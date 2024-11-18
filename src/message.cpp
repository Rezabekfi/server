#include "message.h"
#include "player.h"
#include "quoridor_game.h"
#include <stdexcept>
#include <iostream>

Message::Message() {
    message = nlohmann::ordered_json::object();
    message = {
        {"type", message_type_to_string(MessageType::WRONG_MESSAGE)},
        {"data", nlohmann::ordered_json::object()}
    };
    type = MessageType::WRONG_MESSAGE;
}

Message::Message(const std::string& json_string) {
    try {
        message = nlohmann::ordered_json::parse(json_string);
        type = string_to_message_type(message["type"].get<std::string>());
        if (!message.contains("data")) {
            message = {
                {"type", message_type_to_string(type)},
                {"data", nlohmann::ordered_json::object()}
            };
        }
    } catch (const nlohmann::json::exception& e) {
        type = MessageType::WRONG_MESSAGE;
        message = {
            {"type", message_type_to_string(MessageType::WRONG_MESSAGE)},
            {"data", nlohmann::ordered_json::object()}
        };
    }
}

void Message::set_type(MessageType msg_type) {
    type = msg_type;
    message["type"] = message_type_to_string(type);
}

void Message::set_data(const std::string& key, const std::string& value) {
    message["data"][key] = value;
}

void Message::set_data(const std::string& key, const nlohmann::ordered_json& value) {
    message["data"][key] = value;
}

MessageType Message::get_type() const {
    return type;
}

std::optional<std::string> Message::get_data(const std::string& key) const {
    if (message["data"].contains(key)) {
        return message["data"][key].get<std::string>();
    }
    return std::nullopt;
}

std::optional<nlohmann::ordered_json> Message::get_data_object() const {
    if (message.contains("data")) {
        return message["data"];
    }
    return std::nullopt;
}

std::string Message::to_json() const {
    return message.dump();
}

// TOOD: might need more thorough validation
bool Message::validate() const {
    return message.contains("type") && message.contains("data");
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

// TODO: this might be a useless and replaced by create_next_turn
Message Message::create_game_started(QuoridorGame* game) {
    Message msg = create_next_turn(game);
    msg.set_type(MessageType::GAME_STARTED);
    return msg;
}

Message Message::create_game_ended(QuoridorGame* game, Player* player) {
    Message msg;
    msg.set_type(MessageType::GAME_ENDED);
    msg.set_data("lobby_id", game->get_lobby_id());
    msg.set_data("winner_id", player->id);
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
    msg.set_data("lobby_id", game->get_lobby_id());
    msg.set_data("board", game->get_board_string());
    msg.set_data("current_player_id", game->get_players()[game->get_current_player()]->id);

    // send walls
    msg.set_data("horizontal_walls", game->get_horizontal_walls());
    msg.set_data("vertical_walls", game->get_vertical_walls());
    
    // Add players using the new method
    msg.add_player(game->get_players()[0]);
    msg.add_player(game->get_players()[1]);
    
    return msg;
}

void Message::add_player(Player* player) {
    if (!message["data"].contains("players")) {
        message["data"]["players"] = nlohmann::ordered_json::array();
    }
    
    message["data"]["players"].push_back({
        {"id", player->id},
        {"position", player->position},
        {"name", player->name},
        {"board_char", std::string(1, player->get_board_char())},
        {"walls_left", player->get_walls_left()}
    });
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

Message Message::create_player_disconnected(Player* player) {
    Message msg;
    msg.set_type(MessageType::PLAYER_DISCONNECTED);
    msg.set_data("player_id", player->id);
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
    return MessageType::WRONG_MESSAGE;
}

