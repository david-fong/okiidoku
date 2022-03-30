#include <ookiidoku/db/serdes.hpp>

#include <ookiidoku_util/console_setup.hpp>

#include <string>
#include <iostream>  // cout,

/**
ARGUMENTS
1: grid order (default defined in ookiidoku_config).
2: scramble random key (default: device random number).
*/
int main(const int argc, char const *const argv[]) {
	ookiidoku::util::setup_console();

	unsigned int user_order; // 1
	user_order = (argc > 1) ? std::stoi(argv[1]) : M_OOKIIDOKU_DEFAULT_ORDER;

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order ]] : " << user_order
	<< std::endl;

	// TODO.high

	return 0;
}
