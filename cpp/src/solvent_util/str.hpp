#ifndef HPP_SOLVENT_UTIL__STR
#define HPP_SOLVENT_UTIL__STR

#include <solvent_config.hpp>
#include <locale>
#include <array>
#include <string>

//
namespace solvent::util::str {

	struct MyNumPunct : std::numpunct<char> {
		std::string do_grouping(void) const {
			return "\03";
		}
	};


	struct SgrPair {
		char const*const ON;
		char const*const OFF;
	};
	#if USE_ANSI_ESC
	#define SGR(NAME, ON_STR, OFF_STR) constexpr SgrPair NAME = { .ON = (ON_STR), .OFF = (OFF_STR) };
	#else
	#define SGR(NAME, ON_STR, OFF_STR) constexpr SgrPair NAME = { .ON = "", .OFF = "" };
	#endif

	SGR(DIM, "\033[2m",  "\033[22m")
	SGR(RED, "\033[31m", "\033[39m")

	#undef SGR


	/**
	 * Do not attempt to fill-initialize strings using entries from this
	 * array. The result will not be as expected, which may have to do
	 * with the use of utf-8 strings...
	 *
	 * NOTE: Make sure that the initializer list size matches that
	 * of the corresponding template argument. Compilers won't warn.
	 * See https://cppreference.com/w/cpp/language/sizeof...#Example
	 * for an example utility function I can make to avoid this problem.
	 */
	inline const std::array<std::string, 4> BLOCK_CHARS = {
		#if USE_ANSI_ESC
		"\u2591", "\u2592", "\u2593", "\u2588",
		#else
		"-", "*", "X", "#",
		#endif
	};


	void print_msg_bar(std::string const& msg, unsigned bar_length, std::string fill_char = "═");
	void print_msg_bar(std::string const& msg, std::string fill_char = "═");
}
#endif