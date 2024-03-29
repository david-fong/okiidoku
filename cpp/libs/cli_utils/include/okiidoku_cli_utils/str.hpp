// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_UTILS__STR
#define HPP_OKIIDOKU_CLI_UTILS__STR

// Whether or not ansi-escape sequences should be emitted within _CLI_
// output to dim out harsh, non-essential text, etc.
#define USE_ANSI_ESC true

#include <array>
#include <string_view>
#include <type_traits>
#include <cassert>

namespace okiidoku::util::str {

	struct SgrPair final {
		std::string_view on;
		std::string_view off;
	};
	#if USE_ANSI_ESC
	#define SGR(NAME, ON_STR, OFF_STR) inline constexpr SgrPair NAME { .on {(ON_STR)}, .off {(OFF_STR)} };
	#else
	#define SGR(NAME, ON_STR, OFF_STR) inline constexpr SgrPair NAME { .on {""}, .off {""} };
	#endif

	SGR(dim, "\033[2m",  "\033[22m")
	SGR(red, "\033[31m", "\033[39m")

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
	inline constexpr std::array<std::string_view, 4> box_chars {
		#if USE_ANSI_ESC
		"\u2591", "\u2592", "\u2593", "\u2588",
		#else
		"-", "*", "X", "#",
		#endif
	};
	template<class T>
	requires std::is_arithmetic_v<T>
	constexpr std::string_view get_box_char(const T out_of, const T count) {
		assert(count <= out_of);
		return (count == 0) ? " " : util::str::box_chars[static_cast<std::size_t>(
			(count) * util::str::box_chars.size() / (out_of + 1)
		)];
	}
}
#endif