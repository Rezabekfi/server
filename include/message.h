#pragma once
#include <string>
#include <map>
#include <optional>

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
    
    // Getters
    MessageType get_type() const;
    std::optional<std::string> get_value(const std::string& key) const;
    
    // Conversion
    std::string to_json() const;

    // Validation
    bool validate();

    void set_lobby_id(const std::string& lobby_id);
    void setup_message_type();
    
    // Static factory methods
    static Message create_game_started();
    static Message create_welcome(const std::string& welcome_text);
    static Message create_waiting(const std::string& wait_text);
}; 