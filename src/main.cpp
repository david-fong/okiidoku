#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <clocale>

#include "grid.cpp"

using namespace std;


static locale myLocale;

template<class T>
string formatWithCommas(T value)
{
    stringstream ss;
    ss.imbue(myLocale);
    ss << std::fixed << value;
    return ss.str();
}


/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 */
int main(int argc, char const *const argv[]) {
    cout << endl << "PARSED ARGUMENTS:" << endl;
    order_t userOrder;
    {
        // Arg ONE:
        string arg1 = (argc > 1) ? argv[1] : "4";
        userOrder = stoi(arg1, NULL);
        cout << "- ARG 1 [[ grid order ]] : " << (uint16_t)userOrder << endl;
    } {
        // Arg TWO:
        unsigned int srandKey = (argc > 2)
            ? stoi(argv[2])
            : time(NULL);
        srand(srandKey);
        cout << "- ARG 2 [[ srand key  ]] : " << srandKey << endl;
    }
    cout << endl;

    // Generator loop:
    Game game(userOrder, cout);
    do {
        game.runNew();
        cout << "Time elapsed: " << formatWithCommas(timeDuration.count()) << "us" << endl;
        cout << "Press enter to continue, or anything else to quit." << endl;
    } while (cin.get() == '\n');

    // End of program:
    return 0;
}

