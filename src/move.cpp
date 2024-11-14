#include "move.h"

Move::Move(bool is_horizontal, std::vector<std::pair<int, int>> position) : is_horizontal(is_horizontal), position(position) {}

Move::Move(Message message) {
    nlohmann::json data = message.get_data_object("data").value();
    is_horizontal = data["is_horizontal"].get<bool>();
    position = data["position"].get<std::vector<std::pair<int, int>>>();
}

// this might not be needed TODO: check if necessary
bool Move::get_is_horizontal() const {
    return is_horizontal;
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