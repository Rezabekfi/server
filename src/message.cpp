#include "message.h"
#include <sstream>

Message::Message() {}

Message::Message(const std::string& json_string) {
    size_t pos = 0;
    while ((pos = json_string.find("\"", pos)) != std::string::npos) {
        size_t key_start = pos + 1;
        size_t key_end = json_string.find("\":", pos);
        if (key_end == std::string::npos) break;
        
        std::string key = json_string.substr(key_start, key_end - key_start);
        
        size_t value_start = json_string.find("\"", key_end + 2) + 1;
        size_t value_end = json_string.find("\"", value_start);
        if (value_end == std::string::npos) break;
        
        std::string value = json_string.substr(value_start, value_end - value_start);
        data[key] = value;
        
        pos = value_end + 1;
    }
}

bool Message::validate() {
    return true;
}

void Message::set_type(MessageType type) {
    this->type = type;
}

void Message::set_value(const std::string& key, const std::string& value) {
    data[key] = value;
}

MessageType Message::get_type() const {
    return type;
}

std::optional<std::string> Message::get_value(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string Message::to_json() const {
    std::stringstream ss;
    ss << "{";
    bool first = true;
    for (const auto& [key, value] : data) {
        if (!first) ss << ",";
        ss << "\"" << key << "\":\"" << value << "\"";
        first = false;
    }
    ss << "}";
    return ss.str();
}

// Static factory methods
Message Message::create_game_started() {
    Message msg;
    msg.set_type(MessageType::GAME_STARTED);
    return msg;
}

Message Message::create_welcome(const std::string& welcome_text) {
    Message msg;
    msg.set_type(MessageType::WELCOME);
    msg.set_value("message", welcome_text);
    return msg;
}

Message Message::create_waiting(const std::string& wait_text) {
    Message msg;
    msg.set_type(MessageType::WAITING);
    msg.set_value("message", wait_text);
    return msg;
}

const std::map<MessageType, std::string> type_to_string_map = {
    { GAME_STARTED, "game_started" },
    { WELCOME, "welcome" },
    { WAITING, "waiting" },
    { GAME_ENDED, "game_ended" },
    { MOVE, "move" },
    { MOVE_RESULT, "move_result" },
    { ERROR, "error" },
    { WRONG_MESSAGE, "wrong_message" }
};

const std::map<std::string, MessageType> string_to_type_map = {
    { "game_started", GAME_STARTED },
    { "welcome", WELCOME },
    { "waiting", WAITING },
    { "game_ended", GAME_ENDED },
    { "move", MOVE },
    { "move_result", MOVE_RESULT },
    { "error", ERROR },
    { "wrong_message", WRONG_MESSAGE }
};

// Function implementations
std::string Message::message_type_to_string(MessageType type) {
    auto it = type_to_string_map.find(type);
    return (it != type_to_string_map.end()) ? it->second : "unknown";
}

MessageType Message::string_to_message_type(const std::string& typeStr) {
    auto it = string_to_type_map.find(typeStr);
    return (it != string_to_type_map.end()) ? it->second : WRONG_MESSAGE;
}

void Message::setup_message_type(const std::string& type_str) {
    type = string_to_message_type(type_str);
}