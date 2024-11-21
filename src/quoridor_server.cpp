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

QuoridorServer::QuoridorServer() : game_id_counter(0) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

void QuoridorServer::start(int port) {
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    listen(server_socket, 5);
    std::cout << "Server started on port " << port << std::endl;

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

void QuoridorServer::handle_client(int client_socket) {
    Player* player = initialize_player(client_socket);
    if (!player) return;

    if (!handle_player_name_setup(player)) {
        std::cout << "Player name setup failed for player " << player->name << std::endl;
        cleanup_player(player);
        return;
    }

    if (!handle_matchmaking(player)) {
        std::cout << "Matchmaking failed for player " << player->name << std::endl;
        cleanup_player(player);
        return;
    }

    start_heartbeat_thread(player);
    setup_socket_timeout(client_socket);

    main_client_loop(player);
    std::cout << "Client loop ended for player " << player->name << std::endl;
    cleanup_player(player);
}

Player* QuoridorServer::initialize_player(int client_socket) {
    Player* player = new Player(client_socket);
    player->is_connected = true;
    player->is_reconnecting = false;
    player->update_heartbeat();
    player->send_message(Message::create_welcome("Connected to Quoridor server"));
    player->send_message(Message::create_name_request());
    return player;
}

bool QuoridorServer::handle_player_name_setup(Player* player) {
    char buffer[1024];
    while (true) {
        int bytes_read = recv(player->socket, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) return false;
        
        buffer[bytes_read] = '\0';
        Message msg(buffer);
        
        if (msg.get_type() == MessageType::NAME_RESPONSE) {
            player->set_name(msg.get_data("name").value());
            player->update_heartbeat();
            player->is_connected = true;
            return true;
        } else if (msg.get_type() == MessageType::ACK) {
            continue;
        }
        
        player->send_message(Message::create_error("Please provide your name first"));
        player->send_message(Message::create_name_request());
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
    int bytes_read = recv(player->socket, buffer, sizeof(buffer), 0);
    
    if (bytes_read < 0) {
        return handle_receive_error(player);
    }
    
    buffer[bytes_read] = '\0';
    Message msg(buffer);
    
    player->update_heartbeat();
    
    if (msg.get_type() == MessageType::HEARTBEAT || msg.get_type() == MessageType::ACK) {
        return true;
    }
    std::cout << "Received message: " << msg.to_json() << std::endl;
    
    auto game_it = active_games.find(player->get_game_id());
    if (game_it == active_games.end()) {
        std::cout << "Game not found for player " << player->name << std::endl;
        return false;
    }
    
    return handle_game_message(game_it->second, player, buffer);
}

bool QuoridorServer::handle_receive_error(Player* player) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        return true;  // Timeout occurred, continue the loop
    }
    
    std::cout << "Socket error: " << strerror(errno) << std::endl;
    return false;
}

void QuoridorServer::handle_disconnection(Player* player) {
    auto game_it = active_games.find(player->get_game_id());
    if (game_it != active_games.end()) {
        player->is_connected = false;
        game_it->second->handle_player_disconnection(player);
    }
}

void QuoridorServer::cleanup_player(Player* player) {
    std::cout << "Client disconnected" << std::endl;
    close(player->socket);
    delete player;
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
        std::cout << "ACK received" << std::endl;
        return true;
    }
    Move move(message);
    if (!move.is_valid_structure) {
        player->send_message(Message::create_error("Invalid move structure"));
        return false;
    }
    return true;
}

bool QuoridorServer::handle_game_message(QuoridorGame* game, Player* player, const char* message_string) {
    Message message;
    if (!validate_client_message(game, player, message_string, message)
    || (message.get_type() != MessageType::MOVE && message.get_type() != MessageType::ACK)) {
        return false;
    }

    Move move(message);
    game->handle_move(move);
    return true;
}

QuoridorServer::~QuoridorServer() {
    close(server_socket);
    
    for (auto player : waiting_players) {
        close(player->socket);
        delete player;
    }
    
    for (auto& game_pair : active_games) {
        delete game_pair.second;
    }
}