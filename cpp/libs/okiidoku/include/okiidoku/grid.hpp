#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <array>
#include <span>
#include <compare>
#include <cassert>

namespace okiidoku::mono {

	namespace detail {
		template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) struct Gridlike;
	}
	// contract: all entry values are in the range [0, O2].
	template<Order O> requires(is_order_compiled(O))
	using Grid = detail::Gridlike<O, grid_val_t<O>>;


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_follows_rule(const Grid<O>&) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_filled(const Grid<O>&) noexcept;


	template<Order O, class V_>
	requires(is_order_compiled(O) && !std::is_reference_v<V_>)
	struct detail::Gridlike final { // TODO.mid should this be exported? currently all function body definitions are inline so it can be used header-only... but anything not header-only needs to be exported for sure!
	public:
		using val_t = V_;
		using T = Ints<O>;
		using o2x_t = typename T::o2x_t;
		using o2i_t = typename T::o2i_t;
		using o4x_t = typename T::o4x_t;
		using o4i_t = typename T::o4i_t;
		using array_t = std::array<val_t, T::O4>;

		// lexicographical comparison over row-major-order traversal of cells.
		[[nodiscard, gnu::const]] friend std::strong_ordering operator<=>(const Gridlike<O, V_>& a, const Gridlike<O, V_>& b) noexcept = default;

		// For regular grids, always default initialize as an empty grid (to be safe).
		// Note: Making this constexpr results in a 1% speed gain, but 45% program
		// size increase with GCC. That speed doesn't seem worth it.
		Gridlike() noexcept requires(std::is_same_v<V_, grid_val_t<O>>) {
			arr_.fill(T::O2);
		}
		Gridlike() noexcept requires(!std::is_same_v<V_, grid_val_t<O>>) = default;

		[[nodiscard]]       array_t& get_underlying_array()       noexcept { return arr_; };
		[[nodiscard]] const array_t& get_underlying_array() const noexcept { return arr_; };

		// contract: rmi is in [0, O4).
		template<class T_rmi> requires(Any_o4x<O, T_rmi>)
		[[nodiscard]] constexpr       val_t& at_rmi(const T_rmi rmi)       noexcept { return arr_[rmi]; }
		template<class T_rmi> requires(Any_o4x<O, T_rmi>)
		[[nodiscard]] constexpr const val_t& at_rmi(const T_rmi rmi) const noexcept { return arr_[rmi]; }

		// contract: row and col are in [0, O2).
		template<class T_row, class T_col> requires(Any_o2x<O, T_row> && Any_o2x<O, T_col>)
		[[nodiscard]] constexpr       val_t& at(const T_row row, const T_col col)       noexcept { return arr_[(T::O2*row)+col]; }
		template<class T_row, class T_col> requires(Any_o2x<O, T_row> && Any_o2x<O, T_col>)
		[[nodiscard]] constexpr const val_t& at(const T_row row, const T_col col) const noexcept { return arr_[(T::O2*row)+col]; }

		// contract: row is in [0, O2).
		template<class T_row> requires(Any_o2x<O, T_row>)
		[[nodiscard]] std::span<      val_t, T::O2> row_span_at(const T_row i)       noexcept { return static_cast<std::span<      val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }
		template<class T_row> requires(Any_o2x<O, T_row>)
		[[nodiscard]] std::span<const val_t, T::O2> row_span_at(const T_row i) const noexcept { return static_cast<std::span<const val_t, T::O2>>(std::span(arr_).subspan(T::O2*i, T::O2)); }

		// [[nodiscard]] auto row_spans() noexcept { namespace v = ranges::views; return v::iota(o2i_t{0}, o2i_t{T::O2}) | v::transform([&](auto r){ return row_span_at(r); }); }
		// [[nodiscard]] auto row_spans() const noexcept { namespace v = ranges::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_span_at(r); }); }
	private:
		array_t arr_;
	};

	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_row(const typename Ints<O>::o4i_t index) noexcept { return static_cast<typename Ints<O>::o2i_t>(index / (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_col(const typename Ints<O>::o4i_t index) noexcept { return static_cast<typename Ints<O>::o2i_t>(index % (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_box(const typename Ints<O>::o2i_t row, const typename Ints<O>::o2i_t col) noexcept {
		return static_cast<typename Ints<O>::o2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr typename Ints<O>::o2i_t rmi_to_box(const typename Ints<O>::o4i_t index) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(typename Ints<O>::o4i_t c1, typename Ints<O>::o4i_t c2) noexcept {
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

	namespace detail {
		struct GridAdaptor final {
			static constexpr bool is_borrowtype = false;
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

		// contract: rmi is in [0, O4).
		// [[nodiscard]] common_val_t& at_rmi(const Ints::o4i_t rmi)       noexcept;
		[[nodiscard]] common_val_t at_rmi(const Ints::o4i_t rmi) const noexcept;

		// contract: row and col are in [0, O2).
		// [[nodiscard]] common_val_t& at(const Ints::o2i_t row, const Ints::o2i_t col)       noexcept;
		[[nodiscard]] common_val_t at(const Ints::o2i_t row, const Ints::o2i_t col) const noexcept;
	};
}
#endif