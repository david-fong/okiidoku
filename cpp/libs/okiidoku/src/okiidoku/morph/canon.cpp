// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/canon.hpp>

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/order.hpp>

namespace okiidoku::mono::detail {

	/** implemented in canon_sym.cpp.
	\pre the grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	typename Transformation<O>::sym_map_t canon_sym(Grid<O>&) noexcept;

	/** implemented in canon_pos.cpp.
	\pre the grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canon_pos(Grid<O>&) noexcept;
}
namespace okiidoku::mono {

	// contract: `grid` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canonicalize(Grid<O>& grid) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
		const auto sym_map {detail::canon_sym<O>(grid)};
		auto place_map {detail::canon_pos<O>(grid)};
		// Transformation<O> place_map{}; // TODO.high delete when done canon_pos
		place_map.sym_map = sym_map;
		return place_map;
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template Transformation<O_> canonicalize<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	// contract: `vis_grid` is filled and follows the one rule.
	Transformation canonicalize(Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return static_cast<Transformation>(mono::canonicalize(vis_grid.unchecked_get_mono_exact<O_>()));
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}