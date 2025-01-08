#pragma once
#include <string>
#include <map>
#include <optional>
#include <vector>

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
    NAME_RESPONSE,
    HEARTBEAT,
    PLAYER_DISCONNECTED,
    PLAYER_RECONNECTED,
    ABANDON
};

class Message {
private:
    MessageType type;
    std::map<std::string, std::string> data;

    void add_players(std::vector<Player*> player);
    void add_walls(const std::vector<std::pair<int, int>>& horizontal_walls, bool is_horizontal);

    bool extract_data(const std::string& data_str);
public:
    Message();
    explicit Message(const std::string& message_string);
    
    void set_type(MessageType type);
    void set_data(const std::string& key, const std::string& value);
    
    MessageType get_type() const;
    std::optional<std::string> get_data(const std::string& key) const;
    
    std::string to_string() const;
    bool validate() const;

    // Static factory methods
    static Message create_welcome(const std::string& message);
    static Message create_waiting();
    static Message create_game_started(QuoridorGame* game);
    static Message create_game_ended(QuoridorGame* game, Player* player);
    static Message create_error(const std::string& message);
    static Message create_next_turn(QuoridorGame* game);
    static Message create_name_request();
    static Message create_heartbeat();
    static Message create_player_disconnected(Player* player);
    static Message create_player_reconnected(Player* player);
    static Message create_ack();
    // Type conversion helpers
    static std::string message_type_to_string(MessageType type);
    static MessageType string_to_message_type(const std::string& typeStr);
};