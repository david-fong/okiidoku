// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_INTS_IO
#define HPP_OKIIDOKU_INTS_IO

#include <okiidoku/ints.hpp>

#include <ostream>
#include <cstdint>

template<std::uintmax_t max_, okiidoku::IntKind kind_>
std::ostream& operator<<(std::ostream& os, const okiidoku::Int<max_,kind_>& i) noexcept {
	using val_t = okiidoku::Int<max_,kind_>::val_t;
	if constexpr(std::is_same_v<val_t, unsigned char>) {
		return os << std::uint_fast16_t{i.val()};
	} else {
		return os << i.val();
	}
}
#endif