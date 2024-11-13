#pragma once
#include <map>
#include <vector>
#include <mutex>
#include "QuoridorGame.h"

class QuoridorServer {
private:
    int serverSocket;
    std::vector<Player*> waitingPlayers;
    std::map<int, QuoridorGame*> activeGames;
    std::mutex serverMutex;
    int gameIdCounter;

    void handleClient(int clientSocket);
    void handleGameMessage(Player* player, const char* message);

public:
    QuoridorServer();
    void start(int port);
    ~QuoridorServer();
}; 