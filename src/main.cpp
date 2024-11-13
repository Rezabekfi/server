#include "QuoridorServer.h"
#include <iostream>

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