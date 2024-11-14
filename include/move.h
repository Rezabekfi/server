#pragma once
#include <string>
#include <vector>
#include "message.h"

class Move {
public:
    bool is_horizontal;
    std::vector<std::pair<int, int>> position; // if it is a wall there will be two positions

    Move(bool is_horizontal, std::vector<std::pair<int, int>> position);
    Move(Message message);

    //getters
    bool get_is_horizontal() const;
    std::vector<std::pair<int, int>> get_position() const;

    //setters
    void set_is_horizontal(bool is_horizontal);
    void set_position(std::vector<std::pair<int, int>> position);
}; 