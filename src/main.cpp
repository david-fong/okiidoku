
/**
 * Choose build flags for the Repl and Solver here.
 */
#include "./buildflag.hpp"

#include "./sudoku/solver.cpp"
#include "./sudoku/repl.cpp"

#include <iostream>     // cout,
#include <fstream>      // ofstream,
#include <random>       // random_device,

using Sudoku::Repl;

/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 * 3. output file name.
 */
int main(const int argc, char const *const argv[]) {
    // My implementation specifies this as safe:
    std::ios_base::sync_with_stdio(false);

    unsigned int    userOrder;      // 1
    unsigned int    srandKey;       // 2
    std::string     outFileName;    // 3
    std::ostream*   outStream;      // 3

    userOrder = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_ORDER;
    if (argc > 2 && !std::string(argv[2]).empty()) {
        srandKey = std::stoi(argv[2]);
    } else {
        srandKey = std::random_device()();
    }
    if (argc > 3) {
        // TODO [feat] Handle if file already exists: prompt user for whether to overwrite.
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

    // Scramble the random number generator (std::rand is no longer used):
    // std::srand(srandKey);
    Sudoku::VALUE_RNG.seed(srandKey);

    // Create a Solver of the specified order:
    // (It will automatically enter its REPL).
    // TODO [test] See if allocating on heap can cut down on executable
    // size without visibly impacting performance.
    switch (static_cast<Sudoku::Order>(userOrder)) {
        case 3: { Repl<3> s(*outStream); break; }
        case 4: { Repl<4> s(*outStream); break; }
        case 5: { Repl<5> s(*outStream); break; }
        default:
            std::cout << "\nFAILED:\norder must be one of: [ ";
            for (int i = 3; i <= 5; i++) {
                std::cout << i << ", ";
            }
            std::cout << "]" << std::endl;
            break;
    }

    // End of program:
    std::cout << std::endl << "bye bye!\n" << std::endl;
    return 0;
}
