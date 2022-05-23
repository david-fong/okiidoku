#include <okiidoku/morph/scramble.hpp>

#include <range/v3/algorithm/shuffle.hpp>

#include <array>
#include <algorithm> // shuffle
#include <cassert>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> scramble(Grid<O>& grid, SharedRng& shared_rng) noexcept {
		using T = Ints<O>;
		Transformation<O> t {};
		{
			std::lock_guard lock_guard_{shared_rng.mutex};
			ranges::shuffle(t.label_map, shared_rng.rng);
			ranges::shuffle(t.row_map, shared_rng.rng);
			ranges::shuffle(t.col_map, shared_rng.rng);
			for (int_ts::o1i_t<O> chute {0}; chute < T::O1; ++chute) {
				ranges::shuffle(t.row_map[chute], shared_rng.rng);
				ranges::shuffle(t.col_map[chute], shared_rng.rng);
			}
			// t.post_transpose = static_cast<bool>(shared_rng_() % 2); // TODO
		}
		t.apply_in_place(grid);
		assert(grid_follows_rule<O>(grid));
		return t;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Transformation<O_> scramble<O_>(Grid<O_>&, SharedRng&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	Transformation scramble(Grid& vis_grid, SharedRng& shared_rng) noexcept {
		return std::visit([&](auto& mono_grid) {
			return static_cast<Transformation>(mono::scramble(mono_grid, shared_rng));
		}, vis_grid.get_mono_variant());
	}
}