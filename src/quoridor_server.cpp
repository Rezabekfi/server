#include "quoridor_server.h"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>

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
    Player* player = new Player(client_socket);

    // Send welcome message
    std::string welcome_msg = "{\"type\":\"welcome\",\"message\":\"Connected to Quoridor server\"}";
    player->send_message(welcome_msg);

    // Handle matchmaking
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        if (waiting_players.empty()) {
            std::cout << "No opponent found, adding player to waiting list" << std::endl;
            waiting_players.push_back(player);
            std::string wait_msg = "{\"type\":\"waiting\",\"message\":\"Waiting for opponent...\"}";
            player->send_message(wait_msg);
        } else {
            Player* opponent = waiting_players.back();
            waiting_players.pop_back();

            QuoridorGame* game = new QuoridorGame();
            game->add_player(opponent);
            game->add_player(player);

            int game_id = ++game_id_counter;
            active_games[game_id] = game;
        }
    }

    // Main client loop
    char buffer[1024];
    while (true) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';
        handle_game_message(player, buffer);
    }

    // Cleanup
    close(client_socket);
    delete player;
}

void QuoridorServer::handle_game_message(Player* player, const char* message) {
    // TODO: Implement game message handling
}

QuoridorServer::~QuoridorServer() {
    close(server_socket);
    
    // Clean up any remaining players and games
    for (auto player : waiting_players) {
        close(player->socket);
        delete player;
    }
    
    for (auto& game_pair : active_games) {
        delete game_pair.second;
    }
}