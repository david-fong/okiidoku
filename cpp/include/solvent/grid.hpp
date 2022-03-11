#ifndef HPP_SOLVENT__GRID
#define HPP_SOLVENT__GRID

#include "solvent/size.hpp"
#include "solvent_config.hpp"
#include "solvent_export.h"

#include <array>
#include <span>
#include <cassert>

namespace solvent {

	template<Order O, typename T=size<O>::ord2i_t> using grid_arr_t = std::array<std::array<T, O*O>, O*O>;
	template<Order O, typename T=size<O>::ord2i_t> using grid_const_span_t = std::span<const T, O*O*O*O>;
	template<Order O, typename T=size<O>::ord2i_t> using grid_span_t = std::span<T, O*O*O*O>;

	// A thin wrapper over a span.
	template<Order O, typename T=size<O>::ord2i_t>
	class grid_span2d_t final {
		grid_span_t<O, T> span_;
	public:
		grid_span2d_t(grid_span_t<O, T> span): span_{span} {};
		// contract: row and col must be in [0,O2).
		T& at(size<O>::ord2i_t row, size<O>::ord2i_t col) const noexcept {
			assert(row < O*O && col < O*O);
			return span_[(O*O*row) + col];
		}
	};


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> SOLVENT_EXPORT [[nodiscard]] bool is_sudoku_valid(grid_const_span_t<O>) noexcept;


	template<Order O> SOLVENT_EXPORT [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi_to_row(const typename size<O>::ord4i_t index) noexcept { return static_cast<size<O>::ord2i_t>(index / (O*O)); } // I love c++ :')
	template<Order O> SOLVENT_EXPORT [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi_to_col(const typename size<O>::ord4i_t index) noexcept { return static_cast<size<O>::ord2i_t>(index % (O*O)); } // whyyyyyyyyyy
	template<Order O> SOLVENT_EXPORT [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi_to_blk(const typename size<O>::ord2i_t row, const typename size<O>::ord2i_t col) noexcept {
		return static_cast<size<O>::ord2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	SOLVENT_EXPORT inline constexpr typename size<O>::ord2i_t rmi_to_blk(const typename size<O>::ord4i_t index) noexcept {
		return rmi_to_blk<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}

	template<Order O> SOLVENT_EXPORT [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(typename size<O>::ord4i_t c1, typename size<O>::ord4i_t c2) noexcept {
		return (rmi_to_row<O>(c1) == rmi_to_row<O>(c2))
			||  (rmi_to_col<O>(c1) == rmi_to_col<O>(c2))
			||  (rmi_to_blk<O>(c1) == rmi_to_blk<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
	

	template<Order O>
	struct SOLVENT_EXPORT chute_blk_masks {
		using M = size<O>::O2_mask_least_t;
		using T = std::array<M, O>;
		static constexpr T row = [](){
			T _ {0};
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					_[chute] |= static_cast<M>(1 << ((O*chute) + i));
			}	}
			return _;
		}();
		static constexpr T col = [](){
			T _ {0};
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					_[chute] |= static_cast<M>(1 << ((O*i) + chute));
			}	}
			return _;
		}();
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template bool is_sudoku_valid<O_>(grid_const_span_t<O_>) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif