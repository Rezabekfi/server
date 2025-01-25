#pragma once
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include "quoridor_game.h"


/**
 * @brief QuoridorServer server class that handles all new connections and for each of them creates a new thread for handling the messages
 * Server is started in main.cpp.
 */
class QuoridorServer {
private:

    // Constant for the maximum number of games
    static constexpr size_t MAX_GAMES = 50;

    int server_socket; // server socket
    std::vector<Player*> waiting_players; // players waiting for a match
    std::map<size_t, QuoridorGame*> active_games; // active games
    std::mutex server_mutex; // mutex for thread safety
    size_t game_id_counter; // counter for game ids
    std::atomic<bool> running{true}; // flag for the main server loop

    // Thread for client message handling
    void handle_client(int client_socket);
    // Handles clients messages for the game
    bool handle_game_message(QuoridorGame* game, Player* player, const char* message);
    // Handles client messages for the server (if its for the game it calls handle_game_message)
    bool validate_client_message(QuoridorGame* game, Player* player, const char* message_string, Message& message);

    // Initialize new player
    Player* initialize_player(int client_socket);

    // Handle player name setup
    bool handle_player_name_setup(Player* player);

    // Handle matchmaking (wait/start game)
    bool handle_matchmaking(Player* player);

    // Create a new game once two players are matched
    QuoridorGame* create_game(Player* player1, Player* player2);

    // Start a new thread for sending heartbeats
    void start_heartbeat_thread(Player* player);

    // Set socket timeout
    void setup_socket_timeout(int client_socket);

    // Main client loop is called after player is matched and successfully setup and it is just a loop for receiving messages
    void main_client_loop(Player* player);
    // Handle client message is called in the main client loop and it is used to receive and validate messages
    bool handle_client_message(Player* player);
    // Check if the error ocured during receiving the message and handle it accordingly
    bool handle_receive_error(Player* player);

    // Handle disconnection of a player (send message to the opponent and cleanup)
    void handle_disconnection(Player* player);

    // Cleanup player (close socket and delete player)
    void cleanup_player(Player* player);

    // Find a player with the same name that is disconnected (used for reconnection)
    Player* find_disconnected_player(const std::string& name);

    // Handle player reconnection (if the player with the same name is found)
    bool handle_player_reconnection(Player* new_player, Player* existing_player);

     // Start thread that cleans up finished games
    void start_game_cleaner();

    // Thread for cleaning up finished games
    void cleanup_finished_games();
public:
    // Constructor and destructor
    QuoridorServer();
    ~QuoridorServer();
    // Start the server on the given port 
    void start(int port);
}; 