// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/canon.hpp>

#include <okiidoku/detail/canon_stages.hpp>
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/order.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canonicalize(Grid<O>& grid) noexcept {
		OKIIDOKU_ASSERT(grid_is_filled(grid));
		OKIIDOKU_ASSERT(grid_follows_rule(grid));
		const auto sym_map {detail::canon_sym(grid)};
		auto place_map {detail::canon_pos(grid)};
		place_map.sym_map = sym_map;
		return place_map;
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template Transformation<(O_)> canonicalize<(O_)>(Grid<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	Transformation canonicalize(Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return static_cast<Transformation>(mono::canonicalize(vis_grid.unchecked_get_mono_exact<(O_)>()));
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}