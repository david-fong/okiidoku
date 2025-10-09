// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_UTILS_STR
#define HPP_OKIIDOKU_CLI_UTILS_STR

// Whether or not ansi-escape sequences should be emitted within _CLI_
// output to dim out harsh, non-essential text, etc.
#define USE_ANSI_ESC true

#include <charconv>
#include <string_view>
#include <concepts>

namespace okiidoku::util::str {

	struct [[gnu::designated_init]] SgrPair final {
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


	template<std::integral T>
	std::from_chars_result from_chars(const std::string_view in_str, T& out_num, const unsigned char base = 10u) noexcept {
		// https://en.cppreference.com/w/cpp/utility/from_chars.html
		#if defined(__clang__) or defined(__EMSCRIPTEN__)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
		#endif
		return std::from_chars(in_str.data(), in_str.data()+in_str.size(), out_num, base);
		#if defined(__clang__) or defined(__EMSCRIPTEN__)
		#pragma GCC diagnostic pop
		#endif
	}


	// constexpr std::string_view get_box_char(const std::size_t out_of, const std::size_t count) {
	// 	assert(count <= out_of);
	// 	static constexpr auto box_chars {std::to_array<std::string_view>({
	// 		#if USE_ANSI_ESC
	// 		"\u2591", "\u2592", "\u2593", "\u2588",
	// 		#else
	// 		"-", "*", "X", "#",
	// 		#endif
	// 	})};
	// 	return (count == 0u) ? " " : box_chars[static_cast<std::size_t>(
	// 		(count) * box_chars.size() / (out_of + 1uz)
	// 	)];
	// }
}
#endif