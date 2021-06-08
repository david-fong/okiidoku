#ifndef HPP_SOLVENT_UTIL_ANSI
#define HPP_SOLVENT_UTIL_ANSI

#include <solvent_config.hpp>
#include <array>
#include <string>

/**
 * Yes, I could get some library for this, or I could write a whole
 * library for this, but I'd rather cut down on bloat and dependency
 * since the portion of a full-library that I want is very small.
 */
namespace solvent::util::ansi {

	#if USE_ANSI_ESC
	#define SGR(NAME, ON_STR, OFF_STR) constexpr struct {\
		char const*const ON  = ON_STR;\
		char const*const OFF = OFF_STR;\
	} NAME;
	#else
	#define SGR(NAME, ON_STR, OFF_STR) constexpr struct {\
		char const*const ON  = "";\
		char const*const OFF = "";\
	} NAME;
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
}
#endif