// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/serdes.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <string>    // stoi
#include <iostream>  // cout
#include <cstdlib>   // EXIT_SUCCESS

/**
ARGUMENTS
1: grid order
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	// unsigned int user_order; // 1
	// user_order = std::stoi(argv[1]);

	// std::cout << "\nPARSED ARGUMENTS:"
	// << "\n- ARG 1 [[ grid order ]] : " << user_order
	// << std::endl;

	(void)argc; (void)argv; // TODO

	return EXIT_SUCCESS;
}
