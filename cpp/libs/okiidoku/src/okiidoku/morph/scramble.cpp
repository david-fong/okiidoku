#include <okiidoku/morph/scramble.hpp>

#include <range/v3/algorithm/shuffle.hpp>

#include <array>
#include <cassert>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> scramble(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		Transformation<O> t {};
		{
			using rng_t = std::minstd_rand;
			rng_t rng {static_cast<rng_t::result_type>(rng_seed)};
			ranges::shuffle(t.label_map, rng);
			ranges::shuffle(t.row_map, rng);
			ranges::shuffle(t.col_map, rng);
			for (int_ts::o1i_t<O> chute {0}; chute < T::O1; ++chute) {
				ranges::shuffle(t.row_map[chute], rng);
				ranges::shuffle(t.col_map[chute], rng);
			}
			// t.post_transpose = static_cast<bool>(rng() % 2); // TODO add this back when canonicalize can handle it
		}
		t.apply_in_place(grid);
		assert(grid_follows_rule<O>(grid));
		return t;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Transformation<O_> scramble<O_>(Grid<O_>&, rng_seed_t) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	Transformation scramble(Grid& vis_grid, const rng_seed_t rng_seed) noexcept {
		return std::visit([&](auto& mono_grid) {
			return static_cast<Transformation>(mono::scramble(mono_grid, rng_seed));
		}, vis_grid.get_mono_variant());
	}
}