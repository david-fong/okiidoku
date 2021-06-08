#ifndef HPP_SOLVENT_LIB_GRID
#define HPP_SOLVENT_LIB_GRID

#include ":/lib/size.hpp"
#include ":/util/ansi.hpp"

#include <iosfwd>

namespace solvent::lib {
	//
	template<Order O>
	class Grid {
	 static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));
	 public:
		using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		[[gnu::const]] static constexpr ord2_t get_row(const ord4_t index) noexcept { return index / O2; }
		[[gnu::const]] static constexpr ord2_t get_col(const ord4_t index) noexcept { return index % O2; }
		[[gnu::const]] static constexpr ord2_t get_blk(const ord4_t index) noexcept { return get_blk(get_row(index), get_col(index)); }
		[[gnu::const]] static constexpr ord2_t get_blk(const ord2_t row, const ord2_t col) noexcept {
			return ((row / O1) * O1) + (col / O1);
		}
		[[gnu::const]] static constexpr bool can_coords_see_each_other(ord4_t c1, ord4_t c2) noexcept {
			return (get_row(c1) == get_row(c2))
				||  (get_col(c1) == get_col(c2))
				||  (get_blk(c1) == get_blk(c2));
		}
		// Print a value for a text stream. Aims for visual width of two characters.
		[[gnu::hot]] static std::ostream& print_value(std::ostream&, ord2_t) noexcept;
	};

	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Grid<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif