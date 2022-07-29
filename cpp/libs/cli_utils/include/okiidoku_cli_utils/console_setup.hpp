// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_UTILS__CONSOLE_SETUP
#define HPP_OKIIDOKU_CLI_UTILS__CONSOLE_SETUP

namespace okiidoku::util {
	/**
	Use this to setup console apps made by this project.

	On Windows, this enables VT sequences and changes the input and
	output codepages to UTF-8.
	*/
	[[gnu::cold]] void setup_console();
}
#endif