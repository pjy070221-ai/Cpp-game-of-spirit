#include <iostream>
#include "Game.h"

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Rhythm Game - SFML 3.1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC     - Exit" << std::endl;
    std::cout << "  M       - Menu" << std::endl;
    std::cout << "  R       - Reset" << std::endl;
    std::cout << "  S       - Show Settings" << std::endl;
    std::cout << "  SPACE   - Play/Pause" << std::endl;
    std::cout << "  UP/DOWN - Volume +5% / -5%" << std::endl;
    std::cout << "  PGUP/PGDN - Speed +0.5 / -0.5" << std::endl;
    std::cout << "  F11     - Toggle Fullscreen" << std::endl;
    std::cout << "  D/F/J/K - Press notes" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Game failed to start: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    }
    catch (...) {
        std::cerr << "Game failed to start: Unknown error" << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "Game exited normally!" << std::endl;
    std::cin.get();
    return 0;
}