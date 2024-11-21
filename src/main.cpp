#include "quoridor_server.h"
#include <iostream>
#include <any>

int main() {
    try {
        std::cout << "Starting server... fr fr fr" << std::endl;
        QuoridorServer server;
        server.start(5002);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 