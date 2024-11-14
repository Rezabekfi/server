#include "message.h"
#include <stdexcept>

Message::Message() {
    message = nlohmann::json::object();
    message["data"] = nlohmann::json::object();
}

Message::Message(const std::string& json_string) {
    try {
        message = nlohmann::json::parse(json_string);
        type = string_to_message_type(message["type"].get<std::string>());
        if (!message.contains("data")) {
            message["data"] = nlohmann::json::object();
        }
    } catch (const nlohmann::json::exception& e) {
        type = MessageType::WRONG_MESSAGE;
        message = nlohmann::json::object();
        message["data"] = nlohmann::json::object();
    }
}

void Message::set_type(MessageType msg_type) {
    type = msg_type;
    message["type"] = message_type_to_string(type);
}

void Message::set_data(const std::string& key, const std::string& value) {
    message["data"][key] = value;
}

void Message::set_data(const std::string& key, const nlohmann::json& value) {
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

std::optional<nlohmann::json> Message::get_data_object(const std::string& key) const {
    if (message["data"].contains(key)) {
        return message["data"][key];
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

Message Message::create_game_started(const std::string& lobby_id) {
    Message msg;
    msg.set_type(MessageType::GAME_STARTED);
    msg.set_data("lobby_id", lobby_id);
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
        default: return "unknown";
    }
}

MessageType Message::string_to_message_type(const std::string& typeStr) {
    if (typeStr == "welcome") return MessageType::WELCOME;
    if (typeStr == "waiting") return MessageType::WAITING;
    if (typeStr == "game_started") return MessageType::GAME_STARTED;
    if (typeStr == "game_ended") return MessageType::GAME_ENDED;
    if (typeStr == "move") return MessageType::MOVE;
    if (typeStr == "error") return MessageType::ERROR;
    return MessageType::WRONG_MESSAGE;
}

