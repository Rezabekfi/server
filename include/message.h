#pragma once
#include <string>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>
#include "quoridor_game.h"

enum class MessageType {
    WELCOME,
    WAITING,
    GAME_STARTED,
    GAME_ENDED,
    MOVE,
    ERROR,
    WRONG_MESSAGE
};

class Message {
private:
    nlohmann::json message;
    MessageType type;

public:
    
    Message();
    explicit Message(const std::string& json_string);
    
    void set_type(MessageType type);
    void set_data(const std::string& key, const std::string& value);
    void set_data(const std::string& key, const nlohmann::json& value);
    
    MessageType get_type() const;
    std::optional<std::string> get_data(const std::string& key) const;
    std::optional<nlohmann::json> get_data_object(const std::string& key) const;
    
    std::string to_json() const;
    bool validate() const;

    // Static factory methods
    static Message create_welcome(const std::string& message);
    static Message create_waiting();
    static Message create_game_started(QuoridorGame* game);
    
    
    // Type conversion helpers
    static std::string message_type_to_string(MessageType type);
    static MessageType string_to_message_type(const std::string& typeStr);
}; 