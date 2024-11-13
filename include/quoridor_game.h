#pragma once
#include <vector>
#include <mutex>
#include "player.h"
#include "game_state.h"

class QuoridorGame {
private:
    std::vector<Player*> players;
    std::vector<std::pair<int, int>> walls;
    GameState state;
    int current_player;
    std::mutex game_mutex;

public:
    QuoridorGame();
    bool add_player(Player* player);
    void initialize_game();
    void notify_all_players(const std::string& message_type);
}; 