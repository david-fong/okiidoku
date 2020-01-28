#include "grid.cpp"

using namespace std;


/**
 * 
 */
int main(int argc, char const *const argv[]) {
    string arg1 = (argc > 1) ? argv[1] : "4";
    const order_t userOrder = stoi(arg1, NULL);
    cout << "";
    Game game(userOrder);
    do {
        game.runNew();
        game.print();
        cout << endl << "Press enter to continue, or anything else to quit." << endl;
    } while (cin.get() == '\n');
    return 0;
}

