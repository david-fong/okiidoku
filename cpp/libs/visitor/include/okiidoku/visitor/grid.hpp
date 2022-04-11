#ifndef HPP_OKIIDOKU__VISITOR__GRID
#define HPP_OKIIDOKU__VISITOR__GRID

#include <okiidoku/traits.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <array>
#include <span>
#include <variant>
#include <cassert>

namespace okiidoku::visitor {

	template<Order O, typename V=traits<O>::o2i_smol_t> using grid_arr2d_t = std::array<std::array<V, O*O>, O*O>;
	template<Order O, typename V=traits<O>::o2i_smol_t> using grid_arr_flat_t = std::array<V, O*O*O*O>;
	template<Order O, typename V=traits<O>::o2i_smol_t> using grid_const_span_t = std::span<const V, O*O*O*O>;
	template<Order O, typename V=traits<O>::o2i_smol_t> using grid_span_t = std::span<V, O*O*O*O>;

	using grid_n = std::variant<
	std::monostate
	#define M_OKIIDOKU_TEMPL_TEMPL(O_) ,grid_arr_flat_t<O>
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL
	>;

	// A thin wrapper over a span.
	template<Order O, typename V=traits<O>::o2i_smol_t>
	class GridSpan2D final {
		grid_span_t<O, V> span_;
	public:
		constexpr GridSpan2D(grid_span_t<O, V> span): span_{span} {};
		// contract: row and col must be in [0,O2).
		constexpr V& at(traits<O>::o2i_t row, traits<O>::o2i_t col) const noexcept {
			assert(row < O*O && col < O*O);
			return span_[(O*O*row) + col];
		}
		constexpr std::span<V, O*O> operator[](traits<O>::o2i_t row) const noexcept {
			return static_cast<std::span<V, O*O>>(span_.subspan(O*O * row, O*O));
		}
	};


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	// contract: entries of input are in the range [0, O2]. requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(grid_const_span_t<O>) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	// contract: entries of input are in the range [0, O2]. requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(grid_const_span_t<O>) noexcept;

 OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_row(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index / (O*O)); } OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_col(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index % (O*O)); } OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o2i_t row, const typename traits<O>::o2i_t col) noexcept {
		return static_cast<traits<O>::o2i_t>((row / O) * O) + (col / O);
	} [[nodiscard, gnu::const]]
	OKIIDOKU_EXPORT constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o4i_t index) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}
 OKIIDOKU_EXPORT [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(typename traits<O>::o4i_t c1, typename traits<O>::o4i_t c2) noexcept {
		return (rmi_to_row<O>(c1) == rmi_to_row<O>(c2))
			||  (rmi_to_col<O>(c1) == rmi_to_col<O>(c2))
			||  (rmi_to_box<O>(c1) == rmi_to_box<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
}
#endif