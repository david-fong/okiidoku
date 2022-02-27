#ifndef HPP_SOLVENT_LIB__GRID
#define HPP_SOLVENT_LIB__GRID

#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

#include <vector>
#include <array>
#include <span>
#include <cassert>

namespace solvent::lib {

	template<Order O> using grid_vec_t = std::vector<typename size<O>::ord2i_t>;
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

	template<Order O> [[nodiscard]] grid_vec_t<O> grid_mtx2vec(const grid_arr_t<O>&) noexcept;
	template<Order O> [[nodiscard]] grid_arr_t<O> grid_vec2mtx(const grid_vec_t<O>&) noexcept;


	// Returns true if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> [[nodiscard]] bool is_grid_valid(grid_const_span_t<O>) noexcept;


	template<Order O> [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi2row(const typename size<O>::ord4i_t index) noexcept { return static_cast<size<O>::ord2i_t>(index / (O*O)); } // I love c++ :')
	template<Order O> [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi2col(const typename size<O>::ord4i_t index) noexcept { return static_cast<size<O>::ord2i_t>(index % (O*O)); } // whyyyyyyyyyy
	template<Order O> [[nodiscard, gnu::const]] constexpr typename size<O>::ord2i_t rmi2blk(const typename size<O>::ord2i_t row, const typename size<O>::ord2i_t col) noexcept {
		return static_cast<size<O>::ord2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr typename size<O>::ord2i_t rmi2blk(const typename size<O>::ord4i_t index) noexcept {
		return rmi2blk<O>(rmi2row<O>(index), rmi2col<O>(index));
	}

	template<Order O> [[nodiscard, gnu::const]]
	static constexpr bool cells_share_house(typename size<O>::ord4i_t c1, typename size<O>::ord4i_t c2) noexcept {
		return (rmi2row<O>(c1) == rmi2row<O>(c2))
			||  (rmi2col<O>(c1) == rmi2col<O>(c2))
			||  (rmi2blk<O>(c1) == rmi2blk<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
	

	template<Order O>
	struct chute_blk_masks {
		using M = size<O>::O2_mask_least_t;
		using T = std::array<M, O>;
		static constexpr T row = [](){
			T _ {0};
			for (unsigned chute = 0; chute < O; chute++) {
				for (unsigned i = 0; i < O; i++) {
					_[chute] |= static_cast<M>(1 << ((O*chute) + i));
			}	}
			return _;
		}();
		static constexpr T col = [](){
			T _ {0};
			for (unsigned chute = 0; chute < O; chute++) {
				for (unsigned i = 0; i < O; i++) {
					_[chute] |= static_cast<M>(1 << ((O*i) + chute));
			}	}
			return _;
		}();
	};
}


// extern template class std::vector<std::uint_fast8_t>;
#define M_SOLVENT_TEMPL_TEMPL(O_) \
	extern template class std::array<std::array<typename solvent::size<O_>::ord2i_t, O_*O_>, O_*O_>;
M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
#undef M_SOLVENT_TEMPL_TEMPL
#endif