
#include "grid.h"
using namespace std;

/**
 * 
 */
int main() {
	Game game(3);
	game.init();
	//game.print();

	cout << "say gg or die O_o" << endl;
	string gg;
	getline(cin, gg);
	return 0;
}