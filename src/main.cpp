#include <iostream>
#include <fstream>

#include "grid.cpp"


/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 * 3. output file name.
 */
int main(const int argc, char const *const argv[]) {
    std::cout << std::endl << "sizeof Tile: " << sizeof(Sudoku::Solver::Tile) << " bytes" << std::endl;
    std::cout << std::endl << "PARSED ARGUMENTS:" << std::endl;

    unsigned int    userOrder;      // 1
    unsigned int    srandKey;       // 2
    std::string     outFileName;    // 3
    std::ostream*   outStream;      // 3

    userOrder = std::stoi(((argc > 1) ? argv[1] : "4"), NULL);
    if (argc > 2 && !std::string(argv[2]).empty()) {
        srandKey = std::stoi(argv[2]);
    } else {
        srandKey = std::time(NULL);
    }
    if (argc > 3) {
        // TODO: handle if file already exists: prompt user for whether to overwrite.
        outFileName = argv[3];
        outStream = new std::ofstream(outFileName);
    } else {
        outFileName = "std::cout";
        outStream = &std::cout;
    }

    std::cout << "- ARG 1 [[ grid order  ]] : " << (uint16_t)userOrder << std::endl;
    std::cout << "- ARG 2 [[ srand key   ]] : " << srandKey    << std::endl;
    std::cout << "- ARG 3 [[ output file ]] : " << outFileName << std::endl;

    // Scramble the random number generator:
    std::srand(srandKey);

    // Print help menu:
    std::cout << Sudoku::HELP_MESSAGE << std::endl;

    // Generator loop:
    Sudoku::Solver game(userOrder, *outStream);
    std::string command;
    do {
        std::cout << Sudoku::REPL_PROMPT;
        std::getline(std::cin, command);
    } while (game.runCommand(command));

    // End of program:
    std::cout << std::endl << "bye bye!" << std::endl;
    return 0;
}
