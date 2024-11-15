#pragma once
#include <string>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>

// Forward declarations
class Player;
class QuoridorGame;

enum class MessageType {
    WELCOME,
    WAITING,
    GAME_STARTED,
    GAME_ENDED,
    MOVE,
    ERROR,
    WRONG_MESSAGE,
    ACK,
    NEXT_TURN,
    NAME_REQUEST,
    NAME_RESPONSE
};

class Message {
private:
    nlohmann::ordered_json message;
    MessageType type;

    void add_player(Player* player);
public:
    
    Message();
    explicit Message(const std::string& json_string);
    
    void set_type(MessageType type);
    void set_data(const std::string& key, const std::string& value);
    void set_data(const std::string& key, const nlohmann::ordered_json& value);
    
    MessageType get_type() const;
    std::optional<std::string> get_data(const std::string& key) const;
    std::optional<nlohmann::ordered_json> get_data_object() const;
    
    std::string to_json() const;
    bool validate() const;

    // Static factory methods
    static Message create_welcome(const std::string& message);
    static Message create_waiting();
    static Message create_game_started(QuoridorGame* game);
    static Message create_game_ended(QuoridorGame* game, Player* player);
    static Message create_error(const std::string& message);
    static Message create_next_turn(QuoridorGame* game);
    static Message create_name_request();
    // Type conversion helpers
    static std::string message_type_to_string(MessageType type);
    static MessageType string_to_message_type(const std::string& typeStr);
}; 