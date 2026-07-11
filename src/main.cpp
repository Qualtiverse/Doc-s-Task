#include "GameManager.h"
#include <iostream>

int main() {
    try {
        auto& game = GameManager::Instance();
        
        if (!game.Initialize()) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return -1;
        }
        
        game.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}