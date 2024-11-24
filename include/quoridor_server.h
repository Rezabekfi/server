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

    void handle_player_disconnection(QuoridorGame* game, Player* player);

    Player* initialize_player(int client_socket);
    bool handle_player_name_setup(Player* player);
    bool handle_matchmaking(Player* player);
    QuoridorGame* create_game(Player* player1, Player* player2);
    void start_heartbeat_thread(Player* player);
    void setup_socket_timeout(int client_socket);
    void main_client_loop(Player* player);
    bool handle_client_message(Player* player);
    bool handle_receive_error(Player* player);
    void handle_disconnection(Player* player);
    void cleanup_player(Player* player);

    Player* find_disconnected_player(const std::string& name);
    bool handle_player_reconnection(Player* new_player, Player* existing_player);

public:
    QuoridorServer();
    void start(int port);
    ~QuoridorServer();
}; 