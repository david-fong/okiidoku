
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
        cout << "Enter 'y' to continue, or anything else to quit." << endl;
    } while (getchar() == 'y');
    return 0;
}

