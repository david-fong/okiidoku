#ifndef HPP_SOLVENT_UTIL__CONSOLE_SETUP
#define HPP_SOLVENT_UTIL__CONSOLE_SETUP

namespace solvent::util {
	/**
	Use this to setup console apps made by this project.

	On Windows, this enables VT sequences and changes the input and
	output codepages to UTF-8.
	*/
	[[gnu::cold]] void setup_console();
}
#endif