#include "quoridor_server.h"
#include <iostream>
#include <fstream>
#include "message.h"
#include <any>

int main() {
    try {
        std::ifstream settings_file("../connection_settings.txt");
        if (!settings_file.is_open()) {
            throw std::runtime_error("Could not open connection settings file.");
        }

        std::string address;
        int port;
        settings_file >> address >> port;

        std::cout << "Starting server on port " << port << "..." << std::endl;
        QuoridorServer server;
        server.start(port);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}