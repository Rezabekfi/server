#include "quoridor_game.h"

QuoridorGame::QuoridorGame() : state(GameState::WAITING), current_player(0) {}

bool QuoridorGame::add_player(Player* player) {
    std::lock_guard<std::mutex> lock(game_mutex);
    if (players.size() >= 2) return false;
    
    players.push_back(player);
    if (players.size() == 2) {
        initialize_game();
    }
    return true;
}

void QuoridorGame::initialize_board() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = EMPTY_CELL;
        }
    }
    board[players[0]->position.first][players[0]->position.second] = PLAYER_1_CELL;
    board[players[1]->position.first][players[1]->position.second] = PLAYER_2_CELL;
}

void QuoridorGame::initialize_game() {
    players[1]->position = {4, 0};
    players[0]->position = {4, 8};

    initialize_board();

    state = GameState::IN_PROGRESS;
    notify_all_players(Message::create_game_started(this));
}

void QuoridorGame::notify_all_players(Message message) {
    for (auto player : players) {
        player->send_message(message);
    }
} 

size_t QuoridorGame::get_lobby_id() const {
    return lobby_id;
}

void QuoridorGame::set_lobby_id(size_t lobby_id) {
    this->lobby_id = lobby_id;
}

void QuoridorGame::send_next_turn() {
    notify_all_players(Message::create_next_turn(this));
}
std::string QuoridorGame::get_board_string() const {
    std::string board_string;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board_string += board[i][j];
        }
    }
    return board_string;
}
int QuoridorGame::get_current_player() const {
    return current_player;
}
void QuoridorGame::set_current_player(int current_player) {
    this->current_player = current_player;
}

std::vector<std::pair<int, int>> QuoridorGame::get_horizontal_walls() const {
    return horizontal_walls;
}
std::vector<std::pair<int, int>> QuoridorGame::get_vertical_walls() const {
    return vertical_walls;
}
void QuoridorGame::set_horizontal_walls(const std::vector<std::pair<int, int>>& horizontal_walls) {
    this->horizontal_walls = horizontal_walls;
}
void QuoridorGame::set_vertical_walls(const std::vector<std::pair<int, int>>& vertical_walls) {
    this->vertical_walls = vertical_walls;
}
std::vector<Player*> QuoridorGame::get_players() const {
    return players;
}
void QuoridorGame::set_players(const std::vector<Player*>& players) {
    this->players = players;