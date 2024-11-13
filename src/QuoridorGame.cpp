#include "QuoridorGame.h"

QuoridorGame::QuoridorGame() : state(GameState::WAITING), currentPlayer(0) {}

bool QuoridorGame::addPlayer(Player* player) {
    std::lock_guard<std::mutex> lock(gameMutex);
    if (players.size() >= 2) return false;
    
    players.push_back(player);
    if (players.size() == 2) {
        initializeGame();
    }
    return true;
}

void QuoridorGame::initializeGame() {
    players[0]->position = {4, 0};
    players[1]->position = {4, 8};
    state = GameState::IN_PROGRESS;
    notifyAllPlayers("game_started");
}

void QuoridorGame::notifyAllPlayers(const std::string& messageType) {
    std::string message = "{\"type\":\"" + messageType + "\"}";
    for (auto player : players) {
        player->sendMessage(message);
    }
} 