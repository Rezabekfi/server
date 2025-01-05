#include "move.h"
#include <sstream>

Move::Move(bool is_horizontal, std::vector<std::pair<int, int>> position, int player_id) 
    : is_horizontal(is_horizontal), position(position), player_id(player_id) {}

Move::Move(Message message) {
    if (message.get_type() != MessageType::MOVE) {
        is_valid_structure = false;
        return;
    }
    try {
        auto is_horizontal_opt = message.get_data("is_horizontal");
        auto position_opt = message.get_data("position");
        auto player_id_opt = message.get_data("player_id");

        if (!is_horizontal_opt.has_value() || !position_opt.has_value() || !player_id_opt.has_value()) {
            is_valid_structure = false;
            return;
        }

        is_horizontal = (*is_horizontal_opt == "true");
        player_id = std::stoi(*player_id_opt);

        std::istringstream pos_stream(*position_opt);
        std::string pos_pair;
        while (std::getline(pos_stream, pos_pair, ']')) {
            if (pos_pair.empty()) continue;
            pos_pair = pos_pair.substr(pos_pair.find('[') + 1);
            auto delimiter_pos = pos_pair.find(',');
            if (delimiter_pos != std::string::npos) {
                int row = std::stoi(pos_pair.substr(0, delimiter_pos));
                int col = std::stoi(pos_pair.substr(delimiter_pos + 1));
                position.emplace_back(row, col);
            }
            pos_stream.ignore(1, ','); // Ignore the comma
        }

        is_valid_structure = true;
    } catch (const std::exception&) {
        is_valid_structure = false;
    }
}

bool Move::get_is_horizontal() const {
    return is_horizontal;
}

std::vector<std::pair<int, int>> Move::get_position() const {
    return position;
}

void Move::set_is_horizontal(bool is_horizontal) {
    this->is_horizontal = is_horizontal;
}

void Move::set_position(std::vector<std::pair<int, int>> position) {
    this->position = position;
}

bool Move::is_player_move() const {
    return position.size() == 1;
}

bool Move::get_is_valid_structure() const {
    return is_valid_structure;
}

int Move::get_player_id() const {
    return player_id;
}

void Move::set_player_id(int player_id) {
    this->player_id = player_id;
}