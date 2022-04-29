#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/traits.hpp>
#include <okiidoku/order_templates.hpp>

#include <ranges>
#include <array>
#include <span>
#include <compare>
#include <cassert>

namespace okiidoku::mono::detail {
	template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) struct GridlikeSpan;
	template<Order O, class V> requires(is_order_compiled(O) && !std::is_reference_v<V>) struct GridlikeArr;
}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	using GridSpan = detail::GridlikeSpan<O, default_grid_val_t<O>>;

	template<Order O> requires(is_order_compiled(O))
	using GridConstSpan = detail::GridlikeSpan<O, const default_grid_val_t<O>>;

	template<Order O> requires(is_order_compiled(O))
	using GridArr = detail::GridlikeArr<O, default_grid_val_t<O>>;


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan<O>) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan<O>) noexcept;

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT
	void copy_grid(GridConstSpan<O> src, GridSpan<O> dest) noexcept;

	// lexicographical comparison of the row-major-order grid contents of `a` and `b`.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]]
	std::strong_ordering cmp_grids(GridConstSpan<O> a, GridConstSpan<O> b) noexcept;
}
namespace okiidoku::mono::detail {

	template<Order O, class V_>
	requires(is_order_compiled(O) && !std::is_reference_v<V_>)
	class GridlikeArr final { // TODO.mid should this and the span struct be exported? currently all function body definitions are inline so it can be used header-only... but anything not header-only needs to be exported for sure!
	public:
		using val_t = V_;
		friend GridlikeSpan<O, val_t>;
		friend GridlikeSpan<O, const val_t>;
		using T = okiidoku::mono::traits<O>;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;

		// TODO.high.asap get rid of this? we now have cmp_grids
		[[nodiscard]] constexpr friend auto operator<=>(const GridlikeArr& a, const GridlikeArr& b) noexcept = default;

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

		// contract: row is in [0, O2). o2i_t only used for convenience of caller.
		[[nodiscard]] constexpr std::span<val_t, T::O2> row_at(const o2i_t i) noexcept { return static_cast<std::span<val_t, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }
		// [[nodiscard]] constexpr std::span<const V, T::O2> row_at(const o2i_t i) const noexcept { return static_cast<std::span<V, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }

		[[nodiscard]] constexpr auto rows() noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
		// [[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
	private:
		std::array<val_t, T::O4> cells_;
	};


	template<Order O, class V>
	requires(is_order_compiled(O) && !std::is_reference_v<V>)
	class GridlikeSpan final {
	public:
		using val_t = V;
		friend GridlikeSpan<O, const V>;
		friend void okiidoku::mono::copy_grid<O>(GridConstSpan<O>, GridSpan<O>); // TODO.high.asap can we take out the full namespace qualification on the friend function name?
		friend std::strong_ordering okiidoku::mono::cmp_grids<O>(GridConstSpan<O>, GridConstSpan<O>);
		using T = traits<O>;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;

		GridlikeSpan() = delete;

		explicit constexpr GridlikeSpan(const std::span<V, T::O4> cells) noexcept: cells_(cells) {}

		// create const span from non-const span:
		explicit constexpr GridlikeSpan(const std::span<std::remove_const_t<V>, T::O4> cells) noexcept requires std::is_const_v<V>: cells_(cells) {}

		// Note to self: this correctly does not implicitly allow taking temporaries
		template<class G, std::enable_if_t<std::is_const_v<V> || (std::is_const_v<G> == std::is_const_v<V>), bool> = true>
		requires std::is_same_v<std::remove_cvref_t<G>, GridlikeArr<O, std::remove_const_t<V>>>
		constexpr GridlikeSpan(G& arr) noexcept: cells_(arr.cells_) {}

		constexpr GridlikeSpan(const GridlikeSpan<O, V>& other) noexcept = default;

		// create const span from non-const span:
		constexpr GridlikeSpan(const GridlikeSpan<O, std::remove_const_t<V>>& other) noexcept requires std::is_const_v<V>: cells_(other.cells_) {}

		// contract: coord is in [0, O4).
		template<class T_coord> requires(Any_o4ix<O, T_coord>)
		[[nodiscard]] constexpr val_t& at_row_major(const T_coord coord) const noexcept { return cells_[coord]; }

		// contract: row and col are in [0, O2).
		template<class T_row, class T_col> requires(Any_o2ix<O, T_row> && Any_o2ix<O, T_col>)
		[[nodiscard]] constexpr val_t& at(const T_row row, const T_col col) const noexcept { return cells_[(T::O2*row)+col]; }

		// contract: row is in [0, O2). o2i_t only used for convenience of caller.
		[[nodiscard]] constexpr std::span<val_t, T::O2> row_at(const o2i_t i) const noexcept { return static_cast<std::span<V, T::O2>>(cells_.subspan(T::O2*i, T::O2)); }

		[[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
	private:
		std::span<val_t, T::O4> cells_;
	};

	// I don't know why, but this CTAD guide is required for the
	// auto wrappers for the library's algorithm functions.
	template<Order O, class V>
	GridlikeSpan(GridlikeArr<O, V>&) -> GridlikeSpan<O, V>;
	template<Order O, class V>
	GridlikeSpan(GridlikeArr<O, V>&) -> GridlikeSpan<O, const V>;
	template<Order O, class V>
	GridlikeSpan(const GridlikeArr<O, V>&) -> GridlikeSpan<O, const V>;
}
namespace okiidoku::mono {

	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_row(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index / (traits<O>::O2)); }
	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_col(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index % (traits<O>::O2)); }
	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o2i_t row, const typename traits<O>::o2i_t col) noexcept {
		return static_cast<traits<O>::o2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	OKIIDOKU_EXPORT constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o4i_t index) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}

	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]]
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


namespace okiidoku::visitor::detail {
	template<bool is_const>
	struct GridSpan;
}
namespace okiidoku::visitor {

	using GridSpan = detail::GridSpan<false>;
	using GridConstSpan = detail::GridSpan<true>;

	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan) noexcept;

	// changes the order of `dest` to that of `src` if they are not already the same.
	// TODO.high.asap wait... will this work? The data is laid out by the variant in the GridArr class... does it give any guarantees on layout that would make this actually possible to implement when taking varint spans? same question for the transform stuff.
	OKIIDOKU_EXPORT
	void copy_grid(GridConstSpan src, GridSpan dest) noexcept;

	// lexicographical comparison of the row-major-order grid contents of `a` and `b`.
	// First compares the results of calling `get_order()`.
	OKIIDOKU_EXPORT [[nodiscard]]
	std::strong_ordering cmp_grids(GridConstSpan a, GridConstSpan b) noexcept;
}
namespace okiidoku::visitor::detail {
	struct GridArrAdaptor final {
		static constexpr bool is_ref = false;
		template<Order O>
		using type = mono::GridArr<O>;
	};
}
namespace okiidoku::visitor {

	class OKIIDOKU_EXPORT GridArr final : public detail::ContainerBase<detail::GridArrAdaptor> {
	public:
		using ContainerBase::ContainerBase;
		using common_val_t = default_grid_val_t;

		// Note: the accessors here and in GridSpan are readonly right now, meaning
		// library users can only use mutators defined inside the library. That seems
		// fine. This was done because returning a reference to the mono data would
		// require defining a custom reference type, and I'm currently not in the mood.

		// contract: coord is in [0, O4).
		// [[nodiscard]] common_val_t& at_row_major(const traits::o4i_t coord)       noexcept;
		[[nodiscard]] common_val_t at_row_major(const traits::o4i_t coord) const noexcept;

		// contract: row and col are in [0, O2).
		// [[nodiscard]] common_val_t& at(const traits::o2i_t row, const traits::o2i_t col)       noexcept;
		[[nodiscard]] common_val_t at(const traits::o2i_t row, const traits::o2i_t col) const noexcept;
	};
}
namespace okiidoku::visitor::detail {

	template<bool is_const>
	struct GridSpanAdaptor final {
		static constexpr bool is_ref = true;
		template<Order O>
		using type = std::conditional_t<is_const, mono::GridConstSpan<O>, mono::GridSpan<O>>;
	};

	template<bool is_const>
	class OKIIDOKU_EXPORT GridSpan final : public detail::ContainerBase<GridSpanAdaptor<is_const>> {
		using variant_t = detail::OrderVariantFor<GridSpanAdaptor<is_const>>; // TODO.high.asap delete if unused. ooh... it's actually used in some constructors. but could those be changed to not use it?
	public:
		using ContainerBase<GridSpanAdaptor<is_const>>::ContainerBase;
		// using common_val_t = std::conditional_t<is_const, const default_grid_val_t, default_grid_val_t>; // not used because entry accessors are now readonly and return by value.
		using common_val_t = default_grid_val_t;

		GridSpan(const GridSpan& other) noexcept = default;

		// contract: coord is in [0, O4).
		[[nodiscard]] common_val_t at_row_major(const traits::o4i_t coord) const noexcept;

		// contract: row and col are in [0, O2).
		[[nodiscard]] common_val_t at(const traits::o2i_t row, const traits::o2i_t col) const noexcept;
	};

	// Note: could also have implemented these specializations with c++20 `requires` clauses.
	// I had trouble understanding this whole templates-everywhere business. What I have now
	// seems okay.
	template<>
	class OKIIDOKU_EXPORT GridSpan<true> final : public detail::ContainerBase<GridSpanAdaptor<true>>{
		using ContainerBase<GridSpanAdaptor<true>>::ContainerBase;
	public:
		GridSpan(const GridSpan<false>& other) noexcept;
		template<Order O> explicit GridSpan(mono::GridSpan<O> mono_span) noexcept;
		GridSpan(const GridArr& arr) noexcept;
	};

	template<>
	class OKIIDOKU_EXPORT GridSpan<false> final : public detail::ContainerBase<GridSpanAdaptor<false>>{
		using ContainerBase<GridSpanAdaptor<false>>::ContainerBase;
	public:
		GridSpan(GridArr& arr) noexcept;
	};
}
#endif