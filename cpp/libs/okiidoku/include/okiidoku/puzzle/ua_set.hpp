// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__UNAVOIDABLE_SET
#define HPP_OKIIDOKU__PUZZLE__UNAVOIDABLE_SET

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <vector>
#include <array>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct UaSet4 final {
		std::array<int_ts::o4xs_t<O>, 4> rmis;
	};

	template<Order O> requires(is_order_compiled(O))
	struct MinimalUnavoidableSets final {
		std::vector<UaSet4<O>> ua_set_4s;
	};

	// https://www.sudopedia.org/wiki/Deadly_Pattern
	// contract: grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	MinimalUnavoidableSets<O> find_size_4_minimal_unavoidable_sets(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor {
}
#endif