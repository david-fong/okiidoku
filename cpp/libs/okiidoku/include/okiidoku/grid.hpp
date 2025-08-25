// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <array>
#include <span>
#include <compare>
#include <utility> // forward

namespace okiidoku::mono {

	namespace detail {
		template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) struct Gridlike;
	}
	/**
	a structure for a generic sudoku grid.
	\pre all entry values (contents) are in the range [0, O2].
	\note a numerically encoded value of `O2` symbolizes an empty (non-given/puzzle) cell.
	\note not exported. all public members are defined inline in this header. */
	template<Order O> requires(is_order_compiled(O))
	struct Grid final : public detail::Gridlike<O, grid_val_t<O>> {
		using T = Ints<O>;
		/// \note default initialize as an empty grid (to be safe).
		Grid() noexcept: detail::Gridlike<O, grid_val_t<O>>{T::O2} { }
	};
	// using Grid = detail::Gridlike<O, grid_val_t<O>>;
	// Note: the above commented-out type alias results in large exported, mangled
	//  symbol names from `int_ts`'s heavy usage of `std::conditional_t` (but why?).
	//  Using inheritance gains nice mangled symbol names. Speed seems to suffer a
	//  negligible drop (~1%?) or maybe it doesn't. I am happy with this outcome.


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \return `false` if any cells in a same house contain the same value.
	/// \note can be used with incomplete grids.
	bool grid_follows_rule(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \return `true` if _none_ of the cells are empty (equal to `O2`).
	/// \note does not check if the grid follows the one rule.
	bool grid_is_filled(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \return `true` if _all_ of the cells are empty (equal to `O2`).
	bool grid_is_empty(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT
	/// populates a grid with the contents of the Most Canonical Grid.
	void init_most_canonical_grid(Grid<O>&) noexcept;


	template<Order O, class V_>
	requires(is_order_compiled(O) && !std::is_reference_v<V_>)
	struct detail::Gridlike {
	private:
		using T = Ints<O>;
	public:
		using val_t = V_;
		using array_t = std::array<val_t, T::O4>;

		/// lexicographical comparison over row-major-order traversal of cells.
		[[nodiscard, gnu::pure]] friend std::strong_ordering operator<=>(const Gridlike& a, const Gridlike& b) noexcept = default;

		// Note: Making this constexpr results in a 1% speed gain, but 45% program
		// size increase with GCC. That speed doesn't seem worth it.
		Gridlike(V_ fill_value) noexcept { arr_.fill(fill_value); }
		Gridlike() noexcept = default;

		template<class Self>
		[[nodiscard, gnu::pure]] auto&& get_underlying_array(this Self&& self) noexcept { return std::forward<Self>(self).arr_; }

		// TODO.low why does adding an assumption that the value is lteq T::O2 result in increased code size on clang?
		/// \pre `rmi` is in `[0, O4)`.
		template<class T_rmi, class Self> requires(Any_o4x_t<O, T_rmi>)
		[[nodiscard, gnu::pure]] constexpr auto&& at_rmi(this Self&& self, const T_rmi rmi) noexcept {
			OKIIDOKU_CONTRACT_USE(rmi < T::O4);
			// if constexpr (std::same_as<V_, grid_val_t<O>>) { OKIIDOKU_CONTRACT_USE(arr_[rmi] <= T::O2); }
			return std::forward<Self>(self).arr_[rmi];
		}

		// TODO.low why is using row_col_to_rmi slower than "inlining" the expression here? Is it because of the return-type cast? even adding bounds assumptions seems to increase code size...
		/// \pre `row` and `col` are in `[0, O2)`.
		template<class T_row, class T_col, class Self> requires(Any_o2x_t<O, T_row> && Any_o2x_t<O, T_col>)
		[[nodiscard, gnu::pure]] constexpr auto&& at(this Self&& self, const T_row row, const T_col col) noexcept {
			return std::forward<Self>(self).arr_[static_cast<int_ts::o4x_t<O>>(static_cast<int_ts::o4x_t<O>>(T::O2*row)+col)];
		}

		/**
		see `okiidoku::mono::box_cell_to_rmi`.
		\pre `box` and `box_cell` are in `[0, O2)`. */
		template<class T_house, class T_house_cell, class Self> requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>)
		[[nodiscard, gnu::pure]] constexpr auto&& at_box_cell(this Self&& self, const T_house box, const T_house_cell box_cell) noexcept {
			return std::forward<Self>(self).arr_[box_cell_to_rmi<O>(box, box_cell)];
		}

		/// \pre `row` is in [0, O2).
		template<class T_row> requires(Any_o2x_t<O, T_row>)
		[[nodiscard]] std::span<      val_t, T::O2> row_span_at(const T_row i)       noexcept { return static_cast<std::span<      val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }
		template<class T_row> requires(Any_o2x_t<O, T_row>)
		[[nodiscard]] std::span<const val_t, T::O2> row_span_at(const T_row i) const noexcept { return static_cast<std::span<const val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }

		// [[nodiscard]] auto row_spans() noexcept { namespace v = ::ranges::views; return v::iota(o2i_t{0}, o2i_t{T::O2}) | v::transform([&](auto r){ return row_span_at(r); }); }
		// [[nodiscard]] auto row_spans() const noexcept { namespace v = ::ranges::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_span_at(r); }); }
	private:
		array_t arr_;
	};


	template<Order O> [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(int_ts::o4i_t<O> c1_rmi, int_ts::o4i_t<O> c2_rmi) noexcept {
		return (rmi_to_row<O>(c1_rmi) == rmi_to_row<O>(c2_rmi))
			||  (rmi_to_col<O>(c1_rmi) == rmi_to_col<O>(c2_rmi))
			||  (rmi_to_box<O>(c1_rmi) == rmi_to_box<O>(c2_rmi));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
}


namespace okiidoku::visitor {

	struct Grid; // (see below for definition)

	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \copydoc okiidoku::mono::grid_follows_rule
	bool grid_follows_rule(const Grid&) noexcept;

	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \copydoc okiidoku::mono::grid_is_filled
	bool grid_is_filled(const Grid&) noexcept;

	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \copydoc okiidoku::mono::grid_is_empty
	bool grid_is_empty(const Grid&) noexcept;

	OKIIDOKU_EXPORT
	/// \copydoc okiidoku::mono::init_most_canonical_grid
	void init_most_canonical_grid(Grid&) noexcept;


	namespace detail {
		class GridAdaptor final {
		public:
			static constexpr bool is_borrow_type = false;
			template<Order O>
			using type = mono::Grid<O>;
		};
	}

	/**
	\note it is not recommended to directly make collections containing instances
	of this class. the variant members are currently not heapified, so space usage
	is that of the largest member type. large collections could be space wasteful.
	either prefer streaming, or write a collection wrapper that handles that. */
	// TODO.mid make collection wrapper template that stores the mono version and returns variant version from accessors.
	struct OKIIDOKU_EXPORT Grid final : public detail::ContainerBase<detail::GridAdaptor> {
	public:
		using ContainerBase::ContainerBase;
		using val_t = grid_val_t;

		// Note: the accessors here are readonly right now, meaning
		// library users can only use mutators defined inside the library. That seems
		// fine. This was done because returning a reference to the mono data would
		// require defining a custom reference type, and I'm currently not in the mood.
		// Or we could just take the easy route and make setter methods.

		/// \pre `rmi` is in `[0, O4)`.
		// [[nodiscard]] val_t& at_rmi(const int_ts::o4i_t rmi)       noexcept;
		[[nodiscard, gnu::pure]] val_t at_rmi(const int_ts::o4i_t rmi) const noexcept;

		/// \pre `row` and `col` are in `[0, O2)`.
		// [[nodiscard]] val_t& at(const int_ts::o2i_t row, const int_ts::o2i_t col)       noexcept;
		[[nodiscard, gnu::pure]] val_t at(const int_ts::o2i_t row, const int_ts::o2i_t col) const noexcept;
	};
}
#endif