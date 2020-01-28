
#include "grid.cpp"
using namespace std;

/**
 * 
 */
int main() {
    Game game(4);
    do {
        game.runNew();
        game.print();
        cout << "Press <Enter> to continue, or anything else to quit." << endl;
    } while (getchar() == '\n');
    return 0;
}

