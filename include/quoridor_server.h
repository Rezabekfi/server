#pragma once
#include <map>
#include <vector>
#include <mutex>
#include "quoridor_game.h"

class QuoridorServer {
private:
    int server_socket;
    std::vector<Player*> waiting_players;
    std::map<size_t, QuoridorGame*> active_games;
    std::mutex server_mutex;
    size_t game_id_counter;

    void handle_client(int client_socket);
    // if the client is disconnected return false or if the client sends an invalid message return false -> disconnect client
    bool handle_game_message(QuoridorGame* game, Player* player, const char* message);
    bool validate_client_message(QuoridorGame* game, Player* player, const char* message_string, Message& message);

public:
    QuoridorServer();
    void start(int port);
    ~QuoridorServer();
}; 