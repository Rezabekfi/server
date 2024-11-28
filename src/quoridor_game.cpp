#include "quoridor_game.h"
#include <iostream>
#include <thread>

QuoridorGame::QuoridorGame() : state(GameState::WAITING), current_player(0) {}

QuoridorGame::~QuoridorGame() {
    std::lock_guard<std::mutex> lock(game_mutex);
    state = GameState::ENDED;
    /*
    for (auto player : players) {
        if (player) {
            player->is_connected = false;
            player->is_reconnecting = false;
            if (player->get_game_id() != -1) {
                player->set_game_id(-1);
                delete player;
            }
        }
    }
    players.clear();
    */
}

bool QuoridorGame::add_player(Player* player) {
    std::lock_guard<std::mutex> lock(game_mutex);
    if (players.size() >= 2) return false;
    
    players.push_back(player);
    if (players.size() == 2) {
        initialize_game();
    }
    return true;
}

void QuoridorGame::initialize_board() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = EMPTY_CELL;
        }
    }
    board[players[0]->position.first][players[0]->position.second] = PLAYER_1_CELL;
    board[players[1]->position.first][players[1]->position.second] = PLAYER_2_CELL;
}

void QuoridorGame::initialize_game() {
    initialize_players();
    initialize_board();
    state = GameState::IN_PROGRESS;
    notify_all_players(Message::create_game_started(this));
    send_next_turn();
    start_heartbeat_checker();
}


void QuoridorGame::handle_move(Move move) {
    if (move.get_player_id() != current_player) {
        players[move.get_player_id()]->send_message(Message::create_error("Not your turn"));
        return;
    }
    if (!can_move(move)) {
        players[current_player]->send_message(Message::create_error("Invalid move"));
        return;
    }
    // handle move
    apply_move(move);
    if (check_game_end()) {
        handle_game_end();
        return;
    }
    send_next_turn();
}

void QuoridorGame::handle_game_end() {
    state = GameState::ENDED;
    notify_all_players(Message::create_game_ended(this, players[current_player]));
    for (auto player : players) {
        player->is_connected = false;
        player->is_reconnecting = false;
        player->set_game_id(-1);
    }
}

void QuoridorGame::apply_move(Move move) {
    if (move.is_player_move()) {
        apply_player_move(move);
    } else {
        if (move.get_is_horizontal()) {
            horizontal_walls.push_back(move.get_position()[0]);
            horizontal_walls.push_back(move.get_position()[1]);
        } else {
            vertical_walls.push_back(move.get_position()[0]);
            vertical_walls.push_back(move.get_position()[1]);
        }
        players[current_player]->walls_left--;
    }
    current_player = (current_player + 1) % 2;
}

void QuoridorGame::apply_player_move(Move move) {
    // Get current player position
    int curr_row = players[current_player]->position.first;
    int curr_col = players[current_player]->position.second;
    
    // Get target position
    std::pair<int, int> new_pos = move.get_position()[0];
    
    // Check if target square has another player
    char target_square = board[new_pos.first][new_pos.second];
    if (target_square == PLAYER_1_CELL || target_square == PLAYER_2_CELL) {
        // Move the other player to their starting position
        std::pair<int, int> reset_pos = std::make_pair(
            (target_square == PLAYER_1_CELL) ? BOARD_SIZE - 1 : 0, 
            BOARD_SIZE / 2
        );
        if (new_pos == reset_pos) reset_pos.second++;
        players[target_square - '0' - 1]->set_position(reset_pos);
        board[reset_pos.first][reset_pos.second] = target_square;
    }
    
    // Clear current position
    board[curr_row][curr_col] = EMPTY_CELL;
    
    // Move player to new position
    players[current_player]->set_position(new_pos);
    board[new_pos.first][new_pos.second] = PLAYER_1_CELL + current_player;
}

void QuoridorGame::initialize_players() {
    players[0]->set_position({BOARD_SIZE - 1, BOARD_SIZE / 2});
    players[0]->set_color("red");
    players[0]->set_id("1");
    players[0]->set_board_char(PLAYER_1_CELL);
    players[0]->set_goal_row(0);
    players[0]->set_walls_left(10);

    players[1]->set_position({0, BOARD_SIZE / 2});
    players[1]->set_color("blue");
    players[1]->set_id("2");
    players[1]->set_board_char(PLAYER_2_CELL);
    players[1]->set_goal_row(BOARD_SIZE - 1);
    players[1]->set_walls_left(10);
}

void QuoridorGame::notify_all_players(Message message) {
    for (auto player : players) {
        player->send_message(message);
    }
} 

size_t QuoridorGame::get_lobby_id() const {
    return lobby_id;
}

void QuoridorGame::set_lobby_id(size_t lobby_id) {
    this->lobby_id = lobby_id;
}

void QuoridorGame::send_next_turn() {
    notify_all_players(Message::create_next_turn(this));
}

std::string QuoridorGame::get_board_string() const {
    std::string board_string;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board_string += board[i][j];
        }
    }
    return board_string;
}

int QuoridorGame::get_current_player() const {
    return current_player;
}

void QuoridorGame::set_current_player(int current_player) {
    this->current_player = current_player;
}

std::vector<std::pair<int, int>> QuoridorGame::get_horizontal_walls() const {
    return horizontal_walls;
}

std::vector<std::pair<int, int>> QuoridorGame::get_vertical_walls() const {
    return vertical_walls;
}

void QuoridorGame::set_horizontal_walls(const std::vector<std::pair<int, int>>& horizontal_walls) {
    this->horizontal_walls = horizontal_walls;
}

void QuoridorGame::set_vertical_walls(const std::vector<std::pair<int, int>>& vertical_walls) {
    this->vertical_walls = vertical_walls;
}

std::vector<Player*> QuoridorGame::get_players() const {
    return players;
}

void QuoridorGame::set_players(const std::vector<Player*>& players) {
    this->players = players;
}

bool QuoridorGame::check_game_end() {
    if (state == GameState::ENDED) return true;
    for (auto player : players) {
        if (player->position.first == player->get_goal_row()) {
            return true;
        }
    }
    return false;
}

bool QuoridorGame::can_move(Move move) {
    if (!move.get_is_valid_structure()) return false;

    if (move.is_player_move() && QuoridorGame::is_valid_player_move(move)) {
        return true;
    } else if (!move.is_player_move() && QuoridorGame::is_valid_wall_move(move)) {
        return true;
    }
    return false;
}

bool QuoridorGame::is_valid_player_move(Move move) {
    std::pair<int, int> new_pos = move.get_position()[0];
    int curr_row = -1, curr_col = -1;

    // check if move isnt out of bounds
    if (new_pos.first < 0 || new_pos.first >= BOARD_SIZE || new_pos.second < 0 || new_pos.second >= BOARD_SIZE) return false;
    
    // Find current player position on board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == ('0' + current_player + 1)) {
                curr_row = i;
                curr_col = j;
                break;
            }
        }
        if (curr_row != -1) break;
    }
    if (curr_row == -1 || curr_col == -1) return false;

    // Check if move is exactly 1 square away (no diagonals)
    int row_diff = std::abs(new_pos.first - curr_row);
    int col_diff = std::abs(new_pos.second - curr_col);


    if (((row_diff == 1 && col_diff == 0) || (row_diff == 0 && col_diff == 1)) && !is_wall_between(curr_row, curr_col, new_pos.first, new_pos.second)) {
        return true;
    }
    return false;
}

// bug here in situation where wall is under the player player cannot move to his right 
// if player is on 0, 0 and wall is under him (meaning wall is 00 and 01 but horizontal) he cannot move to 0, 1
bool QuoridorGame::is_wall_between(int row1, int col1, int row2, int col2) {
    // horizontal movement
    bool vertical_blocked = false;
    if (row1 == row2) {
        int wall_row = row1;
        int wall_col = std::min(col1, col2);
        vertical_blocked = std::find(vertical_walls.begin(), vertical_walls.end(), std::make_pair(wall_row, wall_col)) != vertical_walls.end();
    }
    // vertical movement
    bool horizontal_blocked = false;
    if (col1 == col2) {
        int wall_col = col1;
        int wall_row = std::min(row1, row2);
        horizontal_blocked = std::find(horizontal_walls.begin(), horizontal_walls.end(), std::make_pair(wall_row, wall_col)) != horizontal_walls.end();
    }
    return vertical_blocked || horizontal_blocked;
}

bool QuoridorGame::is_valid_wall_move(Move move) {
    std::pair<int, int> wall_1 = move.get_position()[0];
    std::pair<int, int> wall_2 = move.get_position()[1];

    // check if player has wall left
    if (players[current_player]->get_walls_left() <= 0) return false;

    // check if wall is placed next to each other
    if (move.get_is_horizontal() && (wall_1.first != wall_2.first || std::abs(wall_1.second - wall_2.second) != 1)) return false;
    if (!move.get_is_horizontal() && (wall_1.second != wall_2.second || std::abs(wall_1.first - wall_2.first) != 1)) return false;

    // check if wall is already placed - if wall conntains the same pair of points
    if (move.get_is_horizontal()) {
        if (std::find(horizontal_walls.begin(), horizontal_walls.end(), std::make_pair(wall_1.first, wall_1.second)) != horizontal_walls.end() ||
            std::find(horizontal_walls.begin(), horizontal_walls.end(), std::make_pair(wall_2.first, wall_2.second)) != horizontal_walls.end()) {
            return false;
        }
    } else {
        if (std::find(vertical_walls.begin(), vertical_walls.end(), std::make_pair(wall_1.first, wall_1.second)) != vertical_walls.end() ||
            std::find(vertical_walls.begin(), vertical_walls.end(), std::make_pair(wall_2.first, wall_2.second)) != vertical_walls.end()) {
            return false;
        }
    }

    // check if player is blocked comepletely -> if there is no path to the other side of the board (forbidden by rules)
    if (QuoridorGame::is_blocked(move)) return false;

    return true;
}

bool QuoridorGame::is_blocked(Move move) {
    // Save current player
    int saved_player = current_player;
    
    apply_move(move);
    for (auto player : players) {
        if(!QuoridorGame::bfs(player)) {
            current_player = saved_player;
            remove_move(move);
            return true;
        }
    }
    current_player = saved_player;
    remove_move(move);
    return false;
}

// bfs for purpose of checking if player is blocked
bool QuoridorGame::bfs(Player* player) {
    // Store original position
    std::pair<int, int> original_pos = player->position;
    
    // Create visited array and queue for BFS
    bool visited[BOARD_SIZE][BOARD_SIZE] = {false};
    std::queue<std::pair<int, int>> queue;
    
    // Start BFS from current position
    queue.push(player->position);
    visited[player->position.first][player->position.second] = true;
    
    // Possible moves: up, right, down, left
    const int dx[] = {-1, 0, 1, 0};
    const int dy[] = {0, 1, 0, -1};
    
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        
        // If we reached the goal row
        if (current.first == player->get_goal_row()) {
            // Reset position and return success
            player->position = original_pos;
            return true;
        }
        
        // Try all four directions
        for (int i = 0; i < 4; i++) {
            int new_x = current.first + dx[i];
            int new_y = current.second + dy[i];
            
            // Check if new position is within bounds
            if (new_x < 0 || new_x >= BOARD_SIZE || new_y < 0 || new_y >= BOARD_SIZE) {
                continue;
            }
            
            // Check if already visited
            if (visited[new_x][new_y]) {
                continue;
            }
            
            // Check if there's a wall blocking the move
            if (is_wall_between(current.first, current.second, new_x, new_y)) {
                continue;
            }
            
            // Valid move found, add to queue
            queue.push({new_x, new_y});
            visited[new_x][new_y] = true;
        }
    }
    
    // Reset position and return failure - no path found
    player->position = original_pos;
    return false;
}

void QuoridorGame::remove_move(Move move) {
    if (move.get_is_horizontal()) {
        horizontal_walls.erase(std::find(horizontal_walls.begin(), horizontal_walls.end(), move.get_position()[0]));
        horizontal_walls.erase(std::find(horizontal_walls.begin(), horizontal_walls.end(), move.get_position()[1]));
    } else {
        vertical_walls.erase(std::find(vertical_walls.begin(), vertical_walls.end(), move.get_position()[0]));
        vertical_walls.erase(std::find(vertical_walls.begin(), vertical_walls.end(), move.get_position()[1]));
    }
    
    players[current_player]->walls_left++;
}

void QuoridorGame::handle_player_disconnection(Player* player) {
    std::lock_guard<std::mutex> lock(game_mutex);
    
    // Notify remaining player about opponent permanent disconnection
    for (auto p : players) {
        if (p != player && p->is_connected) {
            p->send_message(Message::create_game_ended(this, p));
        }
    }
    
    // Set game state to ended
    state = GameState::ENDED;
}

void QuoridorGame::check_player_connections() {
    for (auto player : players) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - player->last_heartbeat).count();
        
        if (player->is_reconnecting && player->check_connection()) {
            player->is_connected = true;
            player->is_reconnecting = false;
            notify_all_players(Message::create_player_reconnected(player));
            player->send_message(Message::create_next_turn(this));
            continue;
        }

        if (player->is_connected && duration >= Player::NORMAL_HEARTBEAT_TIMEOUT && !player->is_reconnecting) {
            player->is_reconnecting = true;
            // Notify other players about temporary disconnection
            for (auto p : players) {
                if (p != player && p->is_connected) {
                    p->send_message(Message::create_player_disconnected(player));
                }
            }
        }
        
        // Check for permanent disconnection
        if (player->is_reconnecting && 
            duration >= Player::RECONNECTION_HEARTBEAT_TIMEOUT) {
            player->is_connected = false;
            handle_player_disconnection(player);
            return;
        }
    }
}

void QuoridorGame::start_heartbeat_checker() {
    std::thread([this]() {
        while (state == GameState::IN_PROGRESS) {
            check_player_connections();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

GameState QuoridorGame::get_state() const {
    return state;
}