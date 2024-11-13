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