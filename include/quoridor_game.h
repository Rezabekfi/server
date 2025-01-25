#pragma once
#include <vector>
#include <mutex>
#include "player.h"
#include "game_state.h"
#include "message.h"
#include "move.h"


/**
 * @brief Class QuoridorGame represents the game logic for the Quoridor game. It is responsible for handling player moves, game state, and game logic.
 * Quoridor game is created in the server.
 */
class QuoridorGame {
private:
    // Constants
    static constexpr int BOARD_SIZE = 9;
    static constexpr char EMPTY_CELL = 'X';
    static constexpr char PLAYER_1_CELL = '1';
    static constexpr char PLAYER_2_CELL = '2';

    // Variables
    std::vector<Player*> players; // players in the game
    std::vector<std::pair<int, int>> horizontal_walls; // horizontal walls on the board
    std::vector<std::pair<int, int>> vertical_walls; // vertical walls on the board
    char board[BOARD_SIZE][BOARD_SIZE]; // the board represented by a 2D array
    GameState state; // current game state
    int current_player; // index of the current player in the players vector
    std::mutex game_mutex; // mutex for thread safety
    size_t lobby_id; // id of the lobby (not used in the current implementation)

    // initialization methods (used at the beginning of the game)
    void initialize_players();
    void initialize_board();

    // game logic methods
    void apply_player_move(Move move);
    void apply_move(Move move);
    bool check_game_end();
    bool is_valid_player_move(Move move);
    bool is_valid_wall_move(Move move);
    bool is_wall_between(int row1, int col1, int row2, int col2);
    bool is_blocked(Move move);
    // bfs for checking if the player can reach the goal
    bool bfs(Player* player);

    // helper methods
    void remove_move(Move move);

    // checks if all players are connected
    void check_player_connections();

public:
    // Constructor and destructor
    QuoridorGame();
    ~QuoridorGame();


    // add player to the game (used by server)
    bool add_player(Player* player);

    // initialize the game
    void initialize_game();
    
    // notify all players in the game with a message
    void notify_all_players(Message message);

    // handle player disconnection of a player
    void handle_player_disconnection(Player* player);

    // starts a thread to check if the players are connected
    void start_heartbeat_checker();

    // handle player move (called by server) (client thread)
    bool can_move(Move move);
    
    // handle player move (called by server)
    void handle_move(Move move);

    // Send board and current player turn
    void send_next_turn();

    // handle game end (notify all players and set the game state)
    void handle_game_end();


    // getters and setters
    size_t get_lobby_id() const;
    void set_lobby_id(size_t lobby_id);

    //getters and setters
    std::string get_board_string() const;
    int get_current_player() const; 
    void set_current_player(int current_player);
    GameState get_state() const;
    std::vector<std::pair<int, int>> get_horizontal_walls() const;
    std::vector<std::pair<int, int>> get_vertical_walls() const;
    void set_horizontal_walls(const std::vector<std::pair<int, int>>& horizontal_walls);
    void set_vertical_walls(const std::vector<std::pair<int, int>>& vertical_walls);
    std::vector<Player*> get_players() const;
    void set_players(const std::vector<Player*>& players);
    
}; 
