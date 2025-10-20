// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_INTS_IO
#define HPP_OKIIDOKU_INTS_IO

#include <okiidoku/ints.hpp>

#include <ostream>
#include <cstdint>

namespace okiidoku {
::std::ostream& operator<<(::std::ostream& os, const ::okiidoku::detail::bounded_int auto i) {
	using I = decltype(i);
	if constexpr(sizeof(typename I::val_t) == 1uz) {
		os << std::uint_fast16_t{i.val()};
	} else {
		os << i.val();
	}
	os << '/' << I::max; {
		using enum okiidoku::IntKind;
		if constexpr (I::kind == fixed) { os << 'c'; }
		if constexpr (I::kind == small) { os << 's'; }
		if constexpr (I::kind == fast ) { os << 'f'; }
	}
	return os;
}
}
#endif