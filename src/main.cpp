#include <cstdlib>
#include <ctime>
//#include <iostream>

#include "grid.cpp"

using namespace std;


/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 */
int main(int argc, char const *const argv[]) {
    cout << "sizeof Tile: " << sizeof(Game::Tile) << endl;
    cout << endl << "PARSED ARGUMENTS:" << endl;
    order_t userOrder;
    bool isPretty;
    {
        // Arg ONE (order):
        string arg1 = (argc > 1) ? argv[1] : "4";
        userOrder = stoi(arg1, NULL);
        cout << "- ARG 1 [[ grid order ]] : " << (uint16_t)userOrder << endl;
    } {
        // Arg TWO (srand key):
        unsigned int srandKey = (argc > 2) ? stoi(argv[2]) : time(NULL);
        srand(srandKey);
        cout << "- ARG 2 [[ srand key  ]] : " << srandKey << endl;
    } {
        // Arg THREE (pretty output):
        isPretty = false; // TODO
    }
    cout << endl;

    // Generator loop:
    Game game(userOrder, cout, isPretty);
    string command;
    do {
        getline(cin, command);
    } while (game.runCommand(command));

    // End of program:
    cout << endl << "bye bye!" << endl;
    return 0;
}

