#pragma once
#include <vector>
#include <mutex>
#include "player.h"
#include "game_state.h"
#include "message.h"
#include "move.h"

class QuoridorGame {
private:
    static const int BOARD_SIZE = 9;
    static const char EMPTY_CELL = 'X';
    static const char PLAYER_1_CELL = '1';
    static const char PLAYER_2_CELL = '2';

    std::vector<Player*> players;
    std::vector<std::pair<int, int>> horizontal_walls;
    std::vector<std::pair<int, int>> vertical_walls;
    char board[BOARD_SIZE][BOARD_SIZE];
    GameState state;
    int current_player;
    std::mutex game_mutex;
    size_t lobby_id;

    void initialize_players();
    void initialize_board();
    void apply_move(Move move);
    bool check_game_end();
    bool is_valid_player_move(Move move);
    bool is_valid_wall_move(Move move);
    bool is_wall_between(int row1, int col1, int row2, int col2);
    bool is_blocked(Move move);
    bool bfs(Player* player);
    void remove_move(Move move);
    void apply_player_move(Move move);
    void handle_player_disconnection(Player* player);
    void check_player_connections();

public:
    QuoridorGame();
    bool add_player(Player* player);
    void initialize_game();
    void notify_all_players(Message message);
    size_t get_lobby_id() const;
    void set_lobby_id(size_t lobby_id);
    bool can_move(Move move);

    // dont forget to pass the next player turn
    void handle_move(Move move);

    // Send board and current player turn
    void send_next_turn();
    void handle_game_end();

    //getters and setters
    std::string get_board_string() const;
    int get_current_player() const; 
    void set_current_player(int current_player);
    std::vector<std::pair<int, int>> get_horizontal_walls() const;
    std::vector<std::pair<int, int>> get_vertical_walls() const;
    void set_horizontal_walls(const std::vector<std::pair<int, int>>& horizontal_walls);
    void set_vertical_walls(const std::vector<std::pair<int, int>>& vertical_walls);
    std::vector<Player*> get_players() const;
    void set_players(const std::vector<Player*>& players);
    void start_heartbeat_checker();
}; 
