#pragma once
#include <string>
#include <utility>
#include "message.h"
#include <chrono>

/**
 * @brief Class Player represents player inside the game. Player is created as soon as the connection is established.
 * 
 */
class Player {
public:
    int socket; // socket for communication
    std::string name; // player name
    std::pair<int, int> position; // player position on the board
    int walls_left; // number of walls left
    std::string color; // player color
    std::string id; // player id
    int game_id; // game id
    int goal_row; // goal row for the player
    std::chrono::steady_clock::time_point last_heartbeat; // last time the player sent a message
    bool is_connected; // flag for connection status
    bool is_reconnecting; // flag for reconnection status
    char board_char; // character representing the player on the board
    static constexpr int HEARTBEAT_INTERVAL = 5; // seconds
    static constexpr int NORMAL_HEARTBEAT_TIMEOUT = 15; // seconds
    static constexpr int RECONNECTION_HEARTBEAT_TIMEOUT = 120; // 2 minutes to reconnect

    // Constructor
    explicit Player(int sock);

    // Send message to the player
    void send_message(const std::string& message);
    void send_message(const Message& message); // send message object

    // Update heartbeat
    void update_heartbeat();
    // Check if the player is connected
    bool check_connection();

    // Setters and getters
    void set_id(std::string id);
    void set_name(std::string name);
    void set_position(std::pair<int, int> position);
    void set_color(std::string color);
    void set_walls_left(int walls_left);
    void set_goal_row(int goal_row);
    void set_game_id(int game_id);
    std::string get_id() const;
    int get_walls_left() const;
    int get_goal_row() const;
    int get_game_id() const;
    void set_board_char(char board_char);
    char get_board_char() const;
}; 
