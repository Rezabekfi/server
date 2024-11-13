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

void QuoridorGame::initialize_game() {
    players[0]->position = {4, 0};
    players[1]->position = {4, 8};
    state = GameState::IN_PROGRESS;
    notify_all_players("game_started");
}

void QuoridorGame::notify_all_players(const std::string& message_type) {
    std::string message = "{\"type\":\"" + message_type + "\"}";
    for (auto player : players) {
        player->send_message(message);
    }
} 