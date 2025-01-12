#include "quoridor_server.h"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "message.h"
#include "move.h"
#include "quoridor_game.h"
#include "player.h"
#include <fstream>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <algorithm>

QuoridorServer::QuoridorServer() : game_id_counter(0) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

void QuoridorServer::start(int port) {
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    // read address from ../connection_settings.txt
    std::ifstream settings_file("../connection_settings.txt");
    if (!settings_file.is_open()) {
        throw std::runtime_error("Could not open connection settings file.");
    }

    std::string address;
    std::getline(settings_file, address);
    std::string port_str;
    std::getline(settings_file, port_str);
    port = std::stoi(port_str);

    std::cout << "Starting server on port " << port << "..." << std::endl;
    if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address format in connection settings: " + address);
    }
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    listen(server_socket, 5);
    std::cout << "Server started on port " << port << std::endl;

    start_game_cleaner();

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        std::cout << "New connection accepted" << std::endl;
        int flag = 1;
        if (setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
            std::cerr << "Failed to set TCP_NODELAY" << std::endl;
            close(client_socket);
            continue;
        }
        std::thread client_thread(&QuoridorServer::handle_client, this, client_socket);
        client_thread.detach();
    }
}

void QuoridorServer::start_game_cleaner() {
    std::thread([this]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            cleanup_finished_games();
        }
    }).detach();
}

void QuoridorServer::cleanup_finished_games() {
    std::lock_guard<std::mutex> lock(server_mutex);
    std::vector<size_t> games_to_remove;
    
    // First, collect all finished games
    for (const auto& game_pair : active_games) {
        if (game_pair.second->get_state() != GameState::IN_PROGRESS) {
            games_to_remove.push_back(game_pair.first);
        }
    }
    
    // Then remove them
    for (size_t game_id : games_to_remove) {
        delete active_games[game_id];
        active_games.erase(game_id);
    }
}

void QuoridorServer::handle_client(int client_socket) {
    Player* player = initialize_player(client_socket);
    if (!player) {
        return;
    }

    setup_socket_timeout(client_socket);

    if (!handle_player_name_setup(player)) {
        std::cout << "Player name setup failed for player " << player->name << std::endl;
        player->is_connected = false; // hard disconnect
        cleanup_player(player);
        return;
    }

    auto disconnected_player = find_disconnected_player(player->name);
    bool skip_matchmaking = false;

    if (disconnected_player != nullptr) {
        skip_matchmaking = handle_player_reconnection(player, disconnected_player);
    }

    if (!skip_matchmaking) {
        if (!handle_matchmaking(player)) {
            std::cout << "Matchmaking failed for player " << player->name << std::endl;
            player->is_connected = false;
            cleanup_player(player);
            return;
        }
    } else {
        player = disconnected_player;
    }

    main_client_loop(player);
    std::cout << "Client loop ended for player " << player->name << std::endl;
    cleanup_player(player);
}

Player* QuoridorServer::initialize_player(int client_socket) {
    Player* player = new Player(client_socket);
    player->update_heartbeat();
    player->is_connected = true;
    player->is_reconnecting = false;
    player->update_heartbeat();
    player->send_message(Message::create_welcome("Connected to Quoridor server"));
    player->send_message(Message::create_name_request());
    return player;
}

bool QuoridorServer::handle_player_name_setup(Player* player) {
    char buffer[1024];
    std::string message_buffer;
    while (true) {
        if (std::chrono::steady_clock::now() - player->last_heartbeat > std::chrono::seconds(Player::NORMAL_HEARTBEAT_TIMEOUT)) {
            return false;
        }
        player->send_message(Message::create_heartbeat());
        int bytes_read = recv(player->socket, buffer, sizeof(buffer), 0);
        if (bytes_read < 0) {
            if (!handle_receive_error(player)) {
                return false;
            } else {
                continue;
            }
        }
        if (bytes_read == 0) return false;

        player->update_heartbeat();
        
        buffer[bytes_read] = '\0';
        message_buffer += buffer;
        std::stringstream ss(message_buffer);
        std::string message;
        while (std::getline(ss, message, '\n')) {
            if (message.empty()) continue;
            Message msg(message);
            // when message is incorrect we print WRONG_MESSAGE, so we dont need to worry about printing out dangerous data. 
            std::cout << "Received message: " << msg.to_string() << std::endl;
            if (msg.get_type() == MessageType::NAME_RESPONSE) {
                // validate first (name is required)
                if (!msg.get_data("name").has_value()) {
                    player->send_message(Message::create_error("Name is required"));
                    return false;
                }
                player->set_name(msg.get_data("name").value());
                
                // Check for disconnected player first
                auto disconnected_player = find_disconnected_player(player->name);
                if (!disconnected_player && active_games.size() >= MAX_GAMES) {
                    // Only reject if not reconnecting and server is full
                    player->send_message(Message::create_error("Server is full"));
                    return false;
                }
                return true;
            } else if (msg.get_type() == MessageType::ACK) {
                continue;
            } else if (msg.get_type() == MessageType::ABANDON) {
                player->is_connected = false;
                return false;
            } else if (msg.get_type() == MessageType::HEARTBEAT) {
                player->send_message(Message::create_ack());
                continue;
            }
            
            player->send_message(Message::create_error("Wrong message (expected name response)"));
            return false;
        }
        message_buffer = ss.str();
    }
}

bool QuoridorServer::handle_matchmaking(Player* player) {
    std::lock_guard<std::mutex> lock(server_mutex);
    
    if (waiting_players.empty()) {
        waiting_players.push_back(player);
        player->send_message(Message::create_waiting());
        return true;
    }

    Player* opponent = waiting_players.back();
    waiting_players.pop_back();

    QuoridorGame* game = create_game(opponent, player);
    return game != nullptr;
}

QuoridorGame* QuoridorServer::create_game(Player* player1, Player* player2) {
    QuoridorGame* game = new QuoridorGame();
    int game_id = ++game_id_counter;
    
    active_games[game_id] = game;
    game->set_lobby_id(game_id);
    
    player1->set_game_id(game_id);
    player2->set_game_id(game_id);
    
    game->add_player(player1);
    game->add_player(player2);
    
    return game;
}

void QuoridorServer::start_heartbeat_thread(Player* player) {
    std::thread([player]() {
        while (player->is_connected) {
            player->send_message(Message::create_heartbeat());
            std::this_thread::sleep_for(std::chrono::seconds(Player::HEARTBEAT_INTERVAL));
        }
    }).detach();
}

void QuoridorServer::setup_socket_timeout(int client_socket) {
    struct timeval tv{1, 0};  // 1 second timeout
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Failed to set socket timeout" << std::endl;
    }
}

void QuoridorServer::main_client_loop(Player* player) {
    while (player->is_connected) {
        if (!handle_client_message(player)) {
            handle_disconnection(player);
            break;
        }
    }
}

bool QuoridorServer::handle_client_message(Player* player) {
    char buffer[1024];
    std::string message_buffer;
    int bytes_read = recv(player->socket, buffer, sizeof(buffer), 0);
    
    if (bytes_read < 0) {
        return handle_receive_error(player);
    } else if (bytes_read == 0) {
        player->is_connected = false;
        return false;
    }
    
    buffer[bytes_read] = '\0';
    message_buffer += buffer;

    std::stringstream ss(message_buffer);
    std::string message;
    while (std::getline(ss, message, '\n')) {
        if (message.empty()) continue;

        Message msg(message);
        player->update_heartbeat();
        
        if (msg.get_type() == MessageType::ACK) {
            continue;
        }

        if (msg.get_type() == MessageType::HEARTBEAT) {
            player->send_message(Message::create_ack());
            continue;
        }
        // when message is incorrect we print WRONG_MESSAGE, so we dont need to worry about printing out dangerous data.
        // printing is here to avoid clustering print statements.
        std::cout << "Received message: " << msg.to_string() << std::endl;
        
        if (msg.get_type() == MessageType::ABANDON) {
            player->is_connected = false;
            return false;
        }

        auto game_it = active_games.find(player->get_game_id());
        if (game_it == active_games.end()) {
            std::cout << "Game not found for player " << player->name << std::endl;
            player->is_connected = false;
            return false;
        }
        
        if (!handle_game_message(game_it->second, player, message.c_str())) {
            return false;
        }
    }
    message_buffer = ss.str();
    return true;
}

bool QuoridorServer::handle_receive_error(Player* player) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        return true;  // Timeout occurred, continue the loop
    }
    
    std::cout << "Socket error: " << strerror(errno) << std::endl;
    // error so we wont try to reconnect (fuck this guy)
    player->is_connected = false;
    return false;
}

void QuoridorServer::handle_disconnection(Player* player) {
    auto game_it = active_games.find(player->get_game_id());

    if (game_it == active_games.end()) {
        player->is_connected = false; // hard disconnect not in game == (most likely left waiting for players or simillar situation)
    }
    // player is hard disconnected = because of errors or tried to send invalid messages (not allowed)
    // if player is disconected because of network issues, we wont do anything, because checker inside game will handle it
    if (game_it != active_games.end() && !player->is_connected) {
        game_it->second->handle_player_disconnection(player);
    }
}

void QuoridorServer::cleanup_player(Player* player) {
    std::cout << "Client disconnected" << std::endl;
    close(player->socket);
    
    // Remove from waiting queue if present
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        auto it = std::find(waiting_players.begin(), waiting_players.end(), player);
        if (it != waiting_players.end()) {
            waiting_players.erase(it);
            delete player;
            return;
        }
    }

    // Only delete if player is not in a game (game will handle deletion)
    if (!player->is_connected && player->get_game_id() == -1) {
        delete player;
    }
}

bool QuoridorServer::validate_client_message(QuoridorGame* game, Player* player, const char* message_string, Message& message) {
    if (game == nullptr) {
        player->send_message(Message::create_error("Game not found"));
        return false;
    }
    message = Message(message_string);
    if (!message.validate()) {
        player->send_message(Message::create_error("Invalid message"));
        return false;
    }
    if (message.get_type() == MessageType::ACK) {
        return true;
    }
    Move move(message);
    if (!move.is_valid_structure) {
        player->send_message(Message::create_error("Invalid move structure"));
        return false;
    }
    try {
        if (move.get_player_id() + 1 != std::stoi(player->get_id())) {
            player->send_message(Message::create_error("Not your turn"));
            return false;
        }
    } catch (std::exception& e) {
        player->send_message(Message::create_error("Invalid player ID"));
        return false;
    }
    return true;
}

bool QuoridorServer::handle_game_message(QuoridorGame* game, Player* player, const char* message_string) {
    Message message;
    if (!validate_client_message(game, player, message_string, message)
    || (message.get_type() != MessageType::MOVE && message.get_type() != MessageType::ACK)) {
        player->is_connected = false;
        return false;
    }

    Move move(message);
    game->handle_move(move);
    return true;
}

QuoridorServer::~QuoridorServer() {
    running = false;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Give cleaner thread time to finish
    close(server_socket);
    
    for (auto player : waiting_players) {
        close(player->socket);
        delete player;
    }
    
    for (auto& game_pair : active_games) {
        delete game_pair.second;
    }
}

Player* QuoridorServer::find_disconnected_player(const std::string& name) {
    std::lock_guard<std::mutex> lock(server_mutex);
    
    // Check in active games
    for (const auto& game_pair : active_games) {
        if (game_pair.second->get_state() != GameState::IN_PROGRESS) {
            // maybe delete game? TODO: figure out how and when delete finished games
            continue;
        }
        for (Player* player : game_pair.second->get_players()) {
            if (player->name == name) {
                return player;
            }
        }
    }
    return nullptr;
}

bool QuoridorServer::handle_player_reconnection(Player* new_player, Player* existing_player) {
    // Check if there's an existing player to reconnect to
    if (existing_player == nullptr) {
        return false;
    }
    // Transfer the socket and update connection status
    existing_player->socket = new_player->socket;
    existing_player->update_heartbeat();
    
    // Send current game state
    auto game = active_games[existing_player->get_game_id()];
    if (!game || game->get_state() != GameState::IN_PROGRESS) {
        return false;
    }
    existing_player->is_reconnecting = true;
    
    delete new_player;  // Clean up the temporary player object
    return true;
}
