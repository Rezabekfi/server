#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

enum class GameState {
    WAITING,
    IN_PROGRESS,
    FINISHED
};

class Player {
public:
    int socket;
    std::string name;
    std::pair<int, int> position;

    Player(int sock) : socket(sock) {}

    void sendMessage(const std::string& message) {
        send(socket, message.c_str(), message.length(), 0);
    }
};

class QuoridorGame {
private:
    std::vector<Player*> players;
    std::vector<std::pair<int, int>> walls;
    GameState state;
    int currentPlayer;
    std::mutex gameMutex;

public:
    QuoridorGame() : state(GameState::WAITING), currentPlayer(0) {}

    bool addPlayer(Player* player) {
        std::lock_guard<std::mutex> lock(gameMutex);
        if (players.size() >= 2) return false;
        
        players.push_back(player);
        if (players.size() == 2) {
            initializeGame();
        }
        return true;
    }

    void initializeGame() {
        players[0]->position = {4, 0};
        players[1]->position = {4, 8};
        state = GameState::IN_PROGRESS;
        notifyAllPlayers("game_started");
    }

    void notifyAllPlayers(const std::string& messageType) {
        std::string message = "{\"type\":\"" + messageType + "\"}";
        for (auto player : players) {
            player->sendMessage(message);
        }
    }
};

class QuoridorServer {
private:
    int serverSocket;
    std::vector<Player*> waitingPlayers;
    std::map<int, QuoridorGame*> activeGames;
    std::mutex serverMutex;
    int gameIdCounter;

    void handleClient(int clientSocket) {
        Player* player = new Player(clientSocket);

        // Send welcome message
        std::string welcomeMsg = "{\"type\":\"welcome\",\"message\":\"Connected to Quoridor server\"}";
        player->sendMessage(welcomeMsg);

        // Handle matchmaking
        {
            std::lock_guard<std::mutex> lock(serverMutex);
            if (waitingPlayers.empty()) {
                waitingPlayers.push_back(player);
                std::string waitMsg = "{\"type\":\"waiting\",\"message\":\"Waiting for opponent...\"}";
                player->sendMessage(waitMsg);
            } else {
                Player* opponent = waitingPlayers.back();
                waitingPlayers.pop_back();

                QuoridorGame* game = new QuoridorGame();
                game->addPlayer(opponent);
                game->addPlayer(player);

                int gameId = ++gameIdCounter;
                activeGames[gameId] = game;
            }
        }

        // Main client loop
        char buffer[1024];
        while (true) {
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) break;

            buffer[bytesRead] = '\0';
            handleGameMessage(player, buffer);
        }

        // Cleanup
        close(clientSocket);
        delete player;
    }

    void handleGameMessage(Player* player, const char* message) {
        // TODO: Implement game message handling
    }

public:
    QuoridorServer() : gameIdCounter(0) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            throw std::runtime_error("Failed to create socket");
        }
    }

    void start(int port) {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        listen(serverSocket, 5);
        std::cout << "Server started on port " << port << std::endl;

        while (true) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            std::cout << "New connection accepted" << std::endl;
            std::thread clientThread(&QuoridorServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    ~QuoridorServer() {
        close(serverSocket);
    }
};

int main() {
    try {
        QuoridorServer server;
        server.start(5000);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}