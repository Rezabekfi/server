#include "quoridor_server.h"
#include <iostream>

int main() {
    try {
        QuoridorServer server;
        std::cout << "Server started on port 5002" << std::endl;
        server.start(5002);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 