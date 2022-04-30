#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/traits.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <ranges>
#include <array>
#include <span>
#include <compare>
#include <cassert>

namespace okiidoku::mono {

	namespace detail {
		template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) class Gridlike;
	}
	template<Order O> requires(is_order_compiled(O))
	using Grid = detail::Gridlike<O, default_grid_val_t<O>>;


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(const Grid<O>&) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(const Grid<O>&) noexcept;


	template<Order O, class V_>
	requires(is_order_compiled(O) && !std::is_reference_v<V_>)
	class detail::Gridlike final { // TODO.mid should this be exported? currently all function body definitions are inline so it can be used header-only... but anything not header-only needs to be exported for sure!
	public:
		using val_t = V_;
		using T = traits<O>;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;

		// lexicographical comparison over row-major-order traversal of cells.
		friend std::strong_ordering operator<=>(const Gridlike<O, V_>& a, const Gridlike<O, V_>& b) noexcept = default;

		// For regular grids, always default initialize as an empty grid (to be safe).
		// Note: Making this constexpr results in a 1% speed gain, but 45% program
		// size increase. That speed doesn't seem worth it.
		Gridlike() noexcept requires(std::is_same_v<V_, default_grid_val_t<O>>) {
			cells_.fill(T::O2);
		}
		Gridlike() noexcept requires(!std::is_same_v<V_, default_grid_val_t<O>>) = default;

		// contract: coord is in [0, O4).
		template<class T_coord> requires(Any_o4ix<O, T_coord>)
		[[nodiscard]] constexpr       val_t& at_row_major(const T_coord coord)       noexcept { return cells_[coord]; }
		template<class T_coord> requires(Any_o4ix<O, T_coord>)
		[[nodiscard]] constexpr const val_t& at_row_major(const T_coord coord) const noexcept { return cells_[coord]; }

		// contract: row and col are in [0, O2).
		template<class T_row, class T_col> requires(Any_o2ix<O, T_row> && Any_o2ix<O, T_col>)
		[[nodiscard]] constexpr       val_t& at(const T_row row, const T_col col)       noexcept { return cells_[(T::O2*row)+col]; }
		template<class T_row, class T_col> requires(Any_o2ix<O, T_row> && Any_o2ix<O, T_col>)
		[[nodiscard]] constexpr const val_t& at(const T_row row, const T_col col) const noexcept { return cells_[(T::O2*row)+col]; }

		// TODO.mid consider renaming this to `row_span_at`. Would help when users want to use auto. Also the `rows()` method.
		// contract: row is in [0, O2).
		template<class T_row> requires(Any_o2ix<O, T_row>)
		[[nodiscard]] constexpr std::span<      val_t, T::O2> row_at(const T_row i)       noexcept { return static_cast<std::span<      val_t, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }
		template<class T_row> requires(Any_o2ix<O, T_row>)
		[[nodiscard]] constexpr std::span<const val_t, T::O2> row_at(const T_row i) const noexcept { return static_cast<std::span<const val_t, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }

		[[nodiscard]] constexpr auto rows() noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
		// [[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
	private:
		std::array<val_t, T::O4> cells_;
	};

	template<Order O> [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_row(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index / (traits<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_col(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index % (traits<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o2i_t row, const typename traits<O>::o2i_t col) noexcept {
		return static_cast<traits<O>::o2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o4i_t index) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr bool cells_share_house(typename traits<O>::o4i_t c1, typename traits<O>::o4i_t c2) noexcept {
		return (rmi_to_row<O>(c1) == rmi_to_row<O>(c2))
			||  (rmi_to_col<O>(c1) == rmi_to_col<O>(c2))
			||  (rmi_to_box<O>(c1) == rmi_to_box<O>(c2));
	}
	// Note: the compiler optimizes the division/modulus pairs just fine.
	

	template<Order O>
	struct OKIIDOKU_EXPORT chute_box_masks final {
		using M = traits<O>::o2_bits_smol;
		using T = std::array<M, O>;
		static inline const T row {[]{ // TODO.wait re-constexpr this when bitset gets constexpr :/ https://github.com/cplusplus/papers/issues/1087
			T _ {0};
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					_[chute] |= M{1} << ((O*chute) + i);
			}	}
			return _;
		}()};
		static inline const T col {[]{
			T _ {0};
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					_[chute] |= M{1} << ((O*i) + chute);
			}	}
			return _;
		}()};
	};
}


namespace okiidoku::visitor {

	class Grid;

	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(const Grid&) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(const Grid&) noexcept;

	namespace detail {
		struct GridAdaptor final {
			static constexpr bool is_ref = false;
			template<Order O>
			using type = mono::Grid<O>;
		};
	}

	// Note: It is not recommended to directly make collections containing
	// instances of this class.
	// TODO.mid make collection wrapper template that stores the mono version and returns variant version from accessors.
	class OKIIDOKU_EXPORT Grid final : public detail::ContainerBase<detail::GridAdaptor> {
	public:
		using ContainerBase::ContainerBase;
		using common_val_t = default_grid_val_t;

		// Note: the accessors here are readonly right now, meaning
		// library users can only use mutators defined inside the library. That seems
		// fine. This was done because returning a reference to the mono data would
		// require defining a custom reference type, and I'm currently not in the mood.
		// Or we could just take the easy route and make setter methods.

		// contract: coord is in [0, O4).
		// [[nodiscard]] common_val_t& at_row_major(const traits::o4i_t coord)       noexcept;
		[[nodiscard]] common_val_t at_row_major(const traits::o4i_t coord) const noexcept;

		// contract: row and col are in [0, O2).
		// [[nodiscard]] common_val_t& at(const traits::o2i_t row, const traits::o2i_t col)       noexcept;
		[[nodiscard]] common_val_t at(const traits::o2i_t row, const traits::o2i_t col) const noexcept;
	};
}
#endif