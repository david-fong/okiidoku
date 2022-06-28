#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>
#include <okiidoku/detail/contract.hpp>

#include <array>
#include <span>
#include <compare>
#include <cassert>

namespace okiidoku::mono {

	namespace detail {
		template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) struct Gridlike;
	}
	// contract: all entry values (contents) are in the range [0, O2].
	// A numerically encoded value of O2 symbolizes an empty (non-given/puzzle) cell.
	// Note: not exported. All public members are defined inline in this header.
	template<Order O> requires(is_order_compiled(O))
	// using Grid = detail::Gridlike<O, grid_val_t<O>>;
	// Note: the above commented-out type alias results in large exported, mangled
	//  symbol names from `int_ts`'s heavy usage of `std::conditional_t` (but why?).
	//  Using inheritance gains nice mangled symbol names. Speed seems to suffer a
	//  negligible drop (~1%?) or maybe it doesn't. I am happy with this outcome.
	struct Grid final : public detail::Gridlike<O, grid_val_t<O>> {
		Grid() noexcept: detail::Gridlike<O, grid_val_t<O>>{} {};
	};


	// Returns `false` if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_follows_rule(const Grid<O>&) noexcept;

	// Returns `true` if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_filled(const Grid<O>&) noexcept;

	// Returns `true` if all of the cells are empty (equal to O2).
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_empty(const Grid<O>&) noexcept;


	template<Order O, class V_>
	requires(is_order_compiled(O) && !std::is_reference_v<V_>)
	struct detail::Gridlike {
	public:
		using val_t = V_;
		using T = Ints<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using o4x_t = int_ts::o4x_t<O>;
		using o4i_t = int_ts::o4i_t<O>;
		using array_t = std::array<val_t, T::O4>;

		// lexicographical comparison over row-major-order traversal of cells.
		[[nodiscard, gnu::pure]] friend std::strong_ordering operator<=>(const Gridlike& a, const Gridlike& b) noexcept = default;

		// For regular grids, always default initialize as an empty grid (to be safe).
		// Note: Making this constexpr results in a 1% speed gain, but 45% program
		// size increase with GCC. That speed doesn't seem worth it.
		Gridlike() noexcept requires(std::same_as<V_, grid_val_t<O>>) {
			arr_.fill(T::O2);
		}
		Gridlike() noexcept requires(!std::same_as<V_, grid_val_t<O>>) = default;

		[[nodiscard, gnu::pure]]       array_t& get_underlying_array()       noexcept { return arr_; };
		[[nodiscard, gnu::pure]] const array_t& get_underlying_array() const noexcept { return arr_; };

		// TODO.low why does adding an assumption that the value is lteq T::O2 result in increased code size on clang?
		// contract: `rmi` is in [0, O4).
		template<class T_rmi> requires(Any_o4x_t<O, T_rmi>)
		[[nodiscard, gnu::pure]] constexpr       val_t& at_rmi(const T_rmi rmi)       noexcept {
			OKIIDOKU_CONTRACT_TRIVIAL_EVAL(rmi < T::O4);
			// if constexpr (std::same_as<V_, grid_val_t<O>>) { OKIIDOKU_CONTRACT_TRIVIAL_EVAL(arr_[rmi] <= T::O2); }
			return arr_[rmi];
		}
		template<class T_rmi> requires(Any_o4x_t<O, T_rmi>)
		[[nodiscard, gnu::pure]] constexpr const val_t& at_rmi(const T_rmi rmi) const noexcept {
			OKIIDOKU_CONTRACT_TRIVIAL_EVAL(rmi < T::O4);
			// if constexpr (std::same_as<V_, grid_val_t<O>>) { OKIIDOKU_CONTRACT_TRIVIAL_EVAL(arr_[rmi] <= T::O2); }
			return arr_[rmi];
		}

		// TODO.low why is using row_col_to_rmi slower than "inlining" the expression here? Is it because of the return-type cast? even adding bounds assumptions seems to increase code size...
		// contract: `row` and `col` are in [0, O2).
		template<class T_row, class T_col> requires(Any_o2x_t<O, T_row> && Any_o2x_t<O, T_col>)
		[[nodiscard]] constexpr       val_t& at(const T_row row, const T_col col)       noexcept { return arr_[static_cast<o4x_t>(static_cast<o4x_t>(T::O2*row)+col)]; }
		template<class T_row, class T_col> requires(Any_o2x_t<O, T_row> && Any_o2x_t<O, T_col>)
		[[nodiscard]] constexpr const val_t& at(const T_row row, const T_col col) const noexcept { return arr_[static_cast<o4x_t>(static_cast<o4x_t>(T::O2*row)+col)]; }

		// contract: `box` and `box_cell` are in [0, O2).
		template<class T_house, class T_house_cell> requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>)
		[[nodiscard]] constexpr       val_t& at_box_cell(const T_house box, const T_house_cell box_cell)       noexcept { return arr_[box_cell_to_rmi<O>(box, box_cell)]; }
		template<class T_house, class T_house_cell> requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>)
		[[nodiscard]] constexpr const val_t& at_box_cell(const T_house box, const T_house_cell box_cell) const noexcept { return arr_[box_cell_to_rmi<O>(box, box_cell)]; }

		// contract: `row` is in [0, O2).
		template<class T_row> requires(Any_o2x_t<O, T_row>)
		[[nodiscard]] std::span<      val_t, T::O2> row_span_at(const T_row i)       noexcept { return static_cast<std::span<      val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }
		template<class T_row> requires(Any_o2x_t<O, T_row>)
		[[nodiscard]] std::span<const val_t, T::O2> row_span_at(const T_row i) const noexcept { return static_cast<std::span<const val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }

		// [[nodiscard]] auto row_spans() noexcept { namespace v = ranges::views; return v::iota(o2i_t{0}, o2i_t{T::O2}) | v::transform([&](auto r){ return row_span_at(r); }); }
		// [[nodiscard]] auto row_spans() const noexcept { namespace v = ranges::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_span_at(r); }); }
	private:
		array_t arr_;
	};


	template<Order O> [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(int_ts::o4i_t<O> c1, int_ts::o4i_t<O> c2) noexcept {
		return (rmi_to_row<O>(c1) == rmi_to_row<O>(c2))
			||  (rmi_to_col<O>(c1) == rmi_to_col<O>(c2))
			||  (rmi_to_box<O>(c1) == rmi_to_box<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
}


namespace okiidoku::visitor {

	struct Grid;

	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_follows_rule(const Grid&) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_filled(const Grid&) noexcept;

	// Returns true if all of the cells are empty (equal to O2).
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_empty(const Grid&) noexcept;


	namespace detail {
		class GridAdaptor final {
		public:
			static constexpr bool is_borrow_type = false;
			template<Order O>
			using type = mono::Grid<O>;
		};
	}

	// Note: It is not recommended to directly make collections containing
	// instances of this class.
	// TODO.mid make collection wrapper template that stores the mono version and returns variant version from accessors.
	struct OKIIDOKU_EXPORT Grid final : public detail::ContainerBase<detail::GridAdaptor> {
	public:
		using ContainerBase::ContainerBase;
		using common_val_t = grid_val_t;

		// Note: the accessors here are readonly right now, meaning
		// library users can only use mutators defined inside the library. That seems
		// fine. This was done because returning a reference to the mono data would
		// require defining a custom reference type, and I'm currently not in the mood.
		// Or we could just take the easy route and make setter methods.

		// contract: `rmi` is in [0, O4).
		// [[nodiscard]] common_val_t& at_rmi(const int_ts::o4i_t rmi)       noexcept;
		[[nodiscard, gnu::pure]] common_val_t at_rmi(const int_ts::o4i_t rmi) const noexcept;

		// contract: `row` and `col` are in [0, O2).
		// [[nodiscard]] common_val_t& at(const int_ts::o2i_t row, const int_ts::o2i_t col)       noexcept;
		[[nodiscard, gnu::pure]] common_val_t at(const int_ts::o2i_t row, const int_ts::o2i_t col) const noexcept;
	};
}
#endif