#include "solvent/morph/scramble.hpp"
#include "solvent/morph/transform.hpp"
#include "solvent/rng.hpp"

#include <array>
#include <algorithm> // shuffle
#include <cassert>

namespace solvent::morph {

	template<Order O>
	requires (is_order_compiled(O))
	void scramble(const grid_span_t<O> grid) {
		Transformation<O> t {};
		{
			std::lock_guard lock_guard_{shared_mt_rng_mutex_};
			std::ranges::shuffle(t.label_map, shared_mt_rng_);
			std::ranges::shuffle(t.row_map, shared_mt_rng_);
			std::ranges::shuffle(t.col_map, shared_mt_rng_);
			for (size_t chute {0}; chute < O; ++chute) {
				std::ranges::shuffle(t.row_map[chute], shared_mt_rng_);
				std::ranges::shuffle(t.col_map[chute], shared_mt_rng_);
			}
			t.transpose = static_cast<bool>(shared_mt_rng_() % 2);
		}
		t.apply_to(grid);
		assert(is_sudoku_valid<O>(grid));
	}


	// Note: this is currently not used anywhere and has no explicit template expansions.
	template<class T>
	requires std::is_integral_v<T>
	void scramble(Order order, std::span<T> grid) {
		assert(is_order_compiled(order));
		assert(grid.size() >= order*order*order*order);
		switch (order) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: { \
				constexpr unsigned O4 = O_*O_*O_*O_; \
				using val_t = size<O_>::ord2i_least_t; \
				std::array<val_t,O4> grid_resize; \
				for (unsigned i {0}; i < O4; ++i) { grid_resize[i] = static_cast<val_t>(grid[i]); } \
				scramble<O_>(std::span(grid_resize)); \
				for (unsigned i {0}; i < O4; ++i) { grid[i] = static_cast<T>(grid_resize[i]); } \
				break; \
			}
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template void scramble<O_>(const grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}