#pragma once
#include <string>
#include <map>
#include <optional>
#include <unordered_map>

enum MessageType {
    GAME_STARTED,
    WELCOME,
    WAITING,
    GAME_ENDED,
    MOVE,
    MOVE_RESULT,
    ERROR,
    WRONG_MESSAGE
};


class Message {
private:
    std::map<std::string, std::string> data;
    std::string lobby_id;
    MessageType type;

public:
    // Constructors
    Message();
    explicit Message(const std::string& json_string);
    
    // Setters
    void set_type(MessageType type);
    void set_value(const std::string& key, const std::string& value);
    void set_lobby_id(const std::string& lobby_id);
    
    // Getters
    MessageType get_type() const;
    std::optional<std::string> get_value(const std::string& key) const;
    
    // Conversion
    std::string to_json() const;
    MessageType string_to_message_type(const std::string& typeStr);
    std::string message_type_to_string(MessageType type);

    // Validation
    bool validate();

    std::string message_type_to_string(MessageType type);
    MessageType string_to_message_type(const std::string& typeStr);
    void setup_message_type(const std::string& type_str);

    // Static factory methods
    static Message create_game_started();
    static Message create_welcome(const std::string& welcome_text);
    static Message create_waiting(const std::string& wait_text);
}; 