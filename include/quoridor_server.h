#pragma once
#include <map>
#include <vector>
#include <mutex>
#include "quoridor_game.h"

class QuoridorServer {
private:
    int server_socket;
    std::vector<Player*> waiting_players;
    std::map<int, QuoridorGame*> active_games;
    std::mutex server_mutex;
    int game_id_counter;

    void handle_client(int client_socket);
    void handle_game_message(Player* player, const char* message);

public:
    QuoridorServer();
    void start(int port);
    ~QuoridorServer();
}; 