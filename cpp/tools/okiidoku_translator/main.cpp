#include <okiidoku/db/serdes.hpp>

#include <okiidoku_util/console_setup.hpp>

#include <string>
#include <iostream>  // cout,

/**
ARGUMENTS
1: grid order (default defined in okiidoku_config).
2: scramble random key (default: device random number).
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	unsigned int user_order; // 1
	user_order = (argc > 1) ? std::stoi(argv[1]) : M_OKIIDOKU_DEFAULT_ORDER;

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order ]] : " << user_order
	<< std::endl;

	// TODO.high

	return 0;
}
