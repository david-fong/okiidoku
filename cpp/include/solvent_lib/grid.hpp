#ifndef HPP_SOLVENT_LIB__GRID
#define HPP_SOLVENT_LIB__GRID

#include <solvent_lib/size.hpp>

#include <vector>
#include <array>

namespace solvent::lib {

	template<Order O> using grid_vec_t = std::vector<typename size<O>::ord2_t>;
	template<Order O> using grid_mtx_t = std::array<std::array<typename size<O>::ord2_t, O*O>, O*O>;

	template<Order O> grid_vec_t<O> grid_mtx2vec(const grid_mtx_t<O>&) noexcept;
	template<Order O> grid_mtx_t<O> grid_mtx2vec(const grid_vec_t<O>&) noexcept;

	// Returns true if any cell in the same house contain the same value.
	// Can be used for incomplete grids.
	template<Order O> [[gnu::const]] bool is_grid_invalid(const grid_mtx_t<O>&) noexcept;

	template<Order O> [[gnu::const]] constexpr typename size<O>::ord2_t rmi2row(const typename size<O>::ord4_t index) noexcept { return index / (O*O); }
	template<Order O> [[gnu::const]] constexpr typename size<O>::ord2_t rmi2col(const typename size<O>::ord4_t index) noexcept { return index % (O*O); }
	template<Order O> [[gnu::const]] constexpr typename size<O>::ord2_t rmi2blk(const typename size<O>::ord2_t row, const typename size<O>::ord2_t col) noexcept {
		return ((row / O) * O) + (col / O);
	}
	template<Order O> [[gnu::const]] constexpr typename size<O>::ord2_t rmi2blk(const typename size<O>::ord4_t index) noexcept { return rmi2blk<O>(rmi2row<O>(index), rmi2col<O>(index)); }

	template<Order O> [[gnu::const]] static constexpr bool cells_share_house(typename size<O>::ord4_t c1, typename size<O>::ord4_t c2) noexcept {
		return (rmi2row<O>(c1) == rmi2row<O>(c2))
			||  (rmi2col<O>(c1) == rmi2col<O>(c2))
			||  (rmi2blk<O>(c1) == rmi2blk<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
}


extern template class std::vector<std::uint_least8_t>;
#define SOLVENT_TEMPL_TEMPL(O_) \
	extern template class std::array<std::array<typename solvent::size<O_>::ord2_t, O_*O_>, O_*O_>;
SOLVENT_INSTANTIATE_ORDER_TEMPLATES
#undef SOLVENT_TEMPL_TEMPL
#endif