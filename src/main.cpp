#include <chrono>
#include <iomanip>
#include <locale>

#include "grid.cpp"

using namespace std;
using namespace std::chrono;


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
 * 
 */
int main(int argc, char const *const argv[]) {
    // Argument parsing:
    cout << endl << "USAGE: The grid order (in range [3,5]) can be passed in as arg1." << endl << endl;
    string arg1 = (argc > 1) ? argv[1] : "4";
    const order_t userOrder = stoi(arg1, NULL);
    cout << "";

    // Generator loop:
    Game game(userOrder);
    do {
        auto timeStart = high_resolution_clock::now();
        game.runNew();
        auto timeFinish = high_resolution_clock::now();
        auto timeDuration = duration_cast<microseconds>(timeFinish - timeStart);
        game.print();
        cout << "Time elapsed: " << formatWithCommas(timeDuration.count()) << "us" << endl;
        cout << "Press enter to continue, or anything else to quit." << endl;
    } while (cin.get() == '\n');

    // End of program:
    return 0;
}

