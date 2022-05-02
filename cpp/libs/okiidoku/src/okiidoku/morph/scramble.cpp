#include <okiidoku/morph/scramble.hpp>

#include <array>
#include <algorithm> // shuffle
#include <cassert>

namespace okiidoku::mono::morph {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> scramble(Grid<O>& grid, SharedRng& shared_rng) {
		Transformation<O> t {};
		{
			std::lock_guard lock_guard_{shared_rng.mutex};
			std::ranges::shuffle(t.label_map, shared_rng.rng);
			std::ranges::shuffle(t.row_map, shared_rng.rng);
			std::ranges::shuffle(t.col_map, shared_rng.rng);
			for (size_t chute {0}; chute < O; ++chute) {
				std::ranges::shuffle(t.row_map[chute], shared_rng.rng);
				std::ranges::shuffle(t.col_map[chute], shared_rng.rng);
			}
			// t.post_transpose = static_cast<bool>(shared_rng_() % 2); // TODO
		}
		t.apply_in_place(grid);
		assert(grid_follows_rule<O>(grid));
		return t;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Transformation<O_> scramble<O_>(Grid<O_>&, SharedRng&);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::morph {

	Transformation scramble(Grid& vis_grid, SharedRng& shared_rng) {
		return std::visit([&](auto& mono_grid) {
			return static_cast<Transformation>(mono::morph::scramble(mono_grid, shared_rng));
		}, vis_grid.get_mono_variant());
	}
}