#include <iostream>

#include "grid.cpp"


/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 */
int main(int argc, char const *const argv[]) {
    std::cout << "sizeof Tile: " << sizeof(Game::Tile) << std::endl;
    std::cout << std::endl << "PARSED ARGUMENTS:" << std::endl;
    order_t userOrder;
    bool isPretty;
    std::ostream& outStream = std::cout;
    {
        // Arg ONE (order):
        std::string arg1 = (argc > 1) ? argv[1] : "4";
        userOrder = std::stoi(arg1, NULL);
        std::cout << "- ARG 1 [[ grid order ]] : " << (uint16_t)userOrder << std::endl;
    } {
        // Arg TWO (srand key):
        unsigned int srandKey = (argc > 2) ? std::stoi(argv[2]) : time(NULL);
        srand(srandKey);
        std::cout << "- ARG 2 [[ srand key  ]] : " << srandKey << std::endl;
    } {
        // Arg THREE (pretty output):
        isPretty = false; // TODO
    }
    // Print help menu:
    std::cout << Game::HELP_MESSAGE;

    // Generator loop:
    Game game(userOrder, outStream, isPretty);
    std::string command;
    do {
        std::cout << Game::REPL_PROMPT;
        std::getline(std::cin, command);
    } while (game.runCommand(command));

    // End of program:
    std::cout << std::endl << "bye bye!" << std::endl;
    return 0;
}

