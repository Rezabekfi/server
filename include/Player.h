#pragma once
#include <string>
#include <utility>
#include "message.h"

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
}; 