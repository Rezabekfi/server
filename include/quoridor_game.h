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
    static const char EMPTY_CELL = ' ';
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

    void initialize_board();

public:
    QuoridorGame();
    bool add_player(Player* player);
    void initialize_game();
    void notify_all_players(Message message);
    size_t get_lobby_id() const;
    void set_lobby_id(size_t lobby_id);
    bool can_move(Message message);
    bool can_move(Move move);

    // dont forget to pass the next player turn
    void handle_move(Move move);

    // Send board and current player turn
    void send_next_turn();

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
}; 
