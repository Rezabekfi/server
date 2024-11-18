#pragma once
#include <string>
#include <utility>
#include "message.h"
#include <chrono>

class Player {
public:
    int socket;
    std::string name;
    std::pair<int, int> position;
    int walls_left;
    std::string color;
    std::string id;
    int game_id;
    int goal_row;
    std::chrono::steady_clock::time_point last_heartbeat;
    bool is_connected;
    bool is_reconnecting;
    char board_char;
    static const int HEARTBEAT_INTERVAL = 5; // seconds
    static const int NORMAL_HEARTBEAT_TIMEOUT = 15; // seconds
    static const int RECONNECTION_HEARTBEAT_TIMEOUT = 120; // 2 minutes to reconnect

    explicit Player(int sock);

    void set_id(std::string id);
    void set_name(std::string name);
    void set_position(std::pair<int, int> position);
    void set_color(std::string color);
    void set_walls_left(int walls_left);
    void set_goal_row(int goal_row);
    void set_game_id(int game_id);
    int get_walls_left() const;
    int get_goal_row() const;
    int get_game_id() const;
    void send_message(const std::string& message);
    void send_message(const Message& message);
    void update_heartbeat();
    bool check_connection();
    void set_board_char(char board_char);
    char get_board_char() const;
}; 
