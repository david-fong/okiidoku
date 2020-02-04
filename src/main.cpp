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

    std::cout << std::endl << "PARSED ARGUMENTS:" << std::endl;
    std::cout << "- ARG 1 [[ grid order  ]] : " << (uint16_t)userOrder << std::endl;
    std::cout << "- ARG 2 [[ srand key   ]] : " << srandKey    << std::endl;
    std::cout << "- ARG 3 [[ output file ]] : " << outFileName << std::endl;

    // Scramble the random number generator:
    std::srand(srandKey);

    // Create a Solver of the specified order:
    // (It will automatically enter its REPL).
    switch (static_cast<Sudoku::Order>(userOrder)) {
        using namespace Sudoku;
        case TWO:   { Solver<TWO>   game(*outStream); break; }
        case THREE: { Solver<THREE> game(*outStream); break; }
        case FOUR:  { Solver<FOUR>  game(*outStream); break; }
        case FIVE:  { Solver<FIVE>  game(*outStream); break; }
        default:
            std::cout << "order must be " << std::endl;
            break;
    }

    // End of program:
    std::cout << std::endl << "bye bye!" << std::endl;
    return 0;
}
