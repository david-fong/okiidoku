// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_GRID
#define HPP_OKIIDOKU_GRID

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/visitor.hpp>
#include <okiidoku/order.hpp>

#include <array>
#include <span>
#include <compare>
#include <utility>     // forward_like
#include <type_traits> // conditional, is_reference_v

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
	struct Grid : public detail::Gridlike<O, grid_sym_t<O>> {
		using T = Ints<O>;
		/** \note default initialize as an empty grid (to be safe). */
		Grid() noexcept: detail::Gridlike<O, grid_sym_t<O>>{T::O2} {}
	};
	// using Grid = detail::Gridlike<O, grid_sym_t<O>>;
	/* \internal the above commented-out type alias results in large exported, mangled
	symbol names from `ints`'s heavy usage of `std::conditional_t` (but why?).
	Using inheritance gains nice mangled symbol names. Speed seems to suffer a
	negligible drop (~1%?) or maybe it doesn't. I am happy with this outcome. */


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/// \return `false` if any cells in a same house contain the same value (other than `O2`).
	/// \note can be used with incomplete grids.
	bool grid_follows_rule(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/** \return `true` if _none_ of the cells are empty (equal to `O2`).
	\note does not check if the grid follows the one rule. */
	bool grid_is_filled(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/** \return `true` if _all_ of the cells are empty (equal to `O2`). */
	bool grid_is_empty(const Grid<O>&) noexcept;

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT
	/** populates a grid with the contents of the Most Canonical Grid. */
	void init_most_canonical_grid(Grid<O>&) noexcept;


	template<Order O, class CellType>
		requires(is_order_compiled(O) && !std::is_reference_v<CellType>)
	struct detail::Gridlike {
	private:
		using T = Ints<O>;
	public:
		using cell_t = CellType;
		using array_t = std::array<CellType, T::O4>;

		/** lexicographical comparison over row-major-order traversal of cells. */
		[[nodiscard, gnu::pure]] friend std::strong_ordering operator<=>(const Gridlike& a, const Gridlike& b) noexcept = default;

		// note: making this constexpr results in a 1% speed gain, but 45% program
		// size increase with GCC. that speed doesn't seem worth it.
		explicit Gridlike(CellType fill_value) noexcept { arr_.fill(fill_value); }
		Gridlike() noexcept = default;

		[[nodiscard, gnu::pure]]
		decltype(auto) get_underlying_array(this auto&& self) noexcept { return std::forward_like<decltype(self)>(self.arr_); }

		/// \pre `rmi` is in `[0, O4)`.
		[[nodiscard, gnu::pure]] constexpr
		decltype(auto) operator[](this auto&& self, const T::o4x_t rmi) noexcept {
			rmi.check();
			return std::forward_like<decltype(self)>(self.arr_[rmi]);
		}

		/// \pre `row` and `col` are in `[0, O2)`.
		[[nodiscard, gnu::pure]] constexpr
		decltype(auto) operator[](this auto&& self, const T::o2x_t row, const T::o2x_t col) noexcept {
			row.check(); col.check();
			return std::forward_like<decltype(self)>(self.arr_[row_col_to_rmi<O>(row, col)]);
		}

		/**
		see `okiidoku::mono::box_cell_to_rmi`.
		\pre `box` and `box_cell` are in `[0, O2)`. */
		[[nodiscard, gnu::pure]] constexpr
		decltype(auto) at_box_cell(this auto&& self, const T::o2x_t box, const T::o2x_t box_cell) noexcept {
			box.check(); box_cell.check();
			return std::forward_like<decltype(self)>(self.arr_[box_cell_to_rmi<O>(box, box_cell)]);
		}

		/// \pre `row` is in [0, O2).
		[[nodiscard, gnu::pure]] constexpr
		decltype(auto) row_span_at(this auto& self [[clang::lifetimebound]], const T::o2x_t i) noexcept {
			i.check();
			using ret_t = std::remove_reference_t<decltype(self.arr_.front())>;
			// TODO test this. is there dangling?
			return static_cast<std::span<ret_t, T::O2>>(std::span{std::forward_like<decltype(self)>(self).arr_}.subspan(T::O2*i, T::O2));
		}

		// [[nodiscard]] auto row_spans() noexcept [[clang::lifetimebound]] { namespace v = ::ranges::views; return v::iota(o2i_t{0u}, o2i_t{T::O2}) | v::transform([&](auto r){ return row_span_at(r); }); }
		// [[nodiscard]] auto row_spans() const noexcept [[clang::lifetimebound]] { namespace v = ::ranges::views; return v::iota(o2i_t{0u}, T::O2) | v::transform([&](auto r){ return row_span_at(r); }); }
	private:
		array_t arr_;
	};


	template<Order O> [[nodiscard, gnu::const]] constexpr
	bool cells_share_house(
		const typename Ints<O>::o4x_t c1_rmi,
		const typename Ints<O>::o4x_t c2_rmi
	) noexcept {
		c1_rmi.check(); c2_rmi.check();
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
		struct GridAdaptor {
			static constexpr bool is_borrow_type = false;
			template<Order O> using type = mono::Grid<O>;
		};
		static_assert(MonoToVisitorAdaptor<GridAdaptor>);
	}

	/**
	\note it is not recommended to directly make collections containing instances
	of this class. the variant members are currently not heapified, so space usage
	is that of the largest member type. large collections could be space wasteful.
	either prefer streaming, or write a collection wrapper that handles that. */
	// TODO.mid make collection wrapper template that stores the mono version and returns variant version from accessors.
	struct OKIIDOKU_EXPORT Grid : public detail::ContainerBase<detail::GridAdaptor> {
	public:
		using ContainerBase<detail::GridAdaptor>::ContainerBase;
		using sym_t = grid_sym_t;
		explicit Grid(Order O) noexcept: ContainerBase{O} {}

		// Note: the accessors here are readonly right now, meaning
		// library users can only use mutators defined inside the library. That seems
		// fine. This was done because returning a reference to the mono data would
		// require defining a custom reference type, and I'm currently not in the mood.
		// Or we could just take the easy route and make setter methods.

		/// \pre `rmi` is in `[0, O4)`.
		[[nodiscard, gnu::pure]] sym_t operator[](ints::o4x_t rmi) const noexcept;

		/// \pre `row` and `col` are in `[0, O2)`.
		[[nodiscard, gnu::pure]] sym_t operator[](ints::o2x_t row, ints::o2x_t col) const noexcept;
	};
}
#endif