#pragma once
#include <vector>
#include <mutex>
#include "Player.h"
#include "GameState.h"

class QuoridorGame {
private:
    std::vector<Player*> players;
    std::vector<std::pair<int, int>> walls;
    GameState state;
    int currentPlayer;
    std::mutex gameMutex;

public:
    QuoridorGame();
    bool addPlayer(Player* player);
    void initializeGame();
    void notifyAllPlayers(const std::string& messageType);
}; 