#pragma once
#include <string>
#include <vector>
#include "message.h"
/**
 * @brief Move class for storing the move information. It is created usually from Message object.
 * Move class stores information about wall placements and player moves.
 */
class Move {
public:
    int player_id; // will match current player id (0,1) - not the player id from the player object
    bool is_horizontal;
    bool is_valid_structure;
    std::vector<std::pair<int, int>> position; // if it is a wall there will be two positions

    Move(bool is_horizontal, std::vector<std::pair<int, int>> position, int player_id); // currently not used
    // constructor for creating a move from a message
    Move(Message message);

    //getters
    bool get_is_horizontal() const;
    std::vector<std::pair<int, int>> get_position() const;
    bool get_is_valid_structure() const;
    int get_player_id() const;
    //setters
    void set_is_horizontal(bool is_horizontal);
    void set_position(std::vector<std::pair<int, int>> position);
    void set_player_id(int player_id);
    bool is_player_move() const;
}; 