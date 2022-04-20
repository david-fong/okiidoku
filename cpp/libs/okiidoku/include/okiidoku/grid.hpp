#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/traits.hpp>
#include <okiidoku/order_templates.hpp>

#include <ranges>
#include <array>
#include <span>
#include <cassert>


namespace okiidoku::mono {

	template<Order O, class V> requires(is_order_compiled(O)) struct GridSpan;

	template<Order O, class V=default_grid_val_t<O>>
	requires(is_order_compiled(O))
	using GridConstSpan = GridSpan<O, const V>;

	template<Order O, class V_=default_grid_val_t<O>>
	requires(is_order_compiled(O))
	struct GridArr final { // TODO.mid should this and the span struct be exported? currently all function body definitions are inline so it can be used header-only...
		using V = std::decay_t<V_>;
		friend GridSpan<O, V>;
		friend GridConstSpan<O, V>;
		friend GridArr<O> grid_arr_copy_from_span<O>(GridConstSpan<O>);
		using T = traits<O>;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;
		template<Order OO>
		[[nodiscard]] constexpr V& operator[](const o4x_t coord) noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr V& at(const o2x_t row, const o2x_t col) noexcept { return cells_[(T::O2*row)+col]; }
		[[nodiscard]] constexpr std::span<V, T::O2> row_at(const o2x_t i) noexcept { return static_cast<std::span<V, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }
		[[nodiscard]] constexpr const V& operator[](const o4x_t coord) const noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr const V& at(const o2x_t row, const o2x_t col) const noexcept { return cells_[(T::O2*row)+col]; }
		// [[nodiscard]] constexpr std::span<const V, T::O2> row_at(const o2x_t i) const noexcept { return static_cast<std::span<V, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }
		// [[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
		[[nodiscard]] constexpr auto rows() noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
	private:
		std::array<V, T::O4> cells_;
	};

	template<Order O>
	requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]] GridArr<O> grid_arr_copy_from_span(GridConstSpan<O> src) noexcept;

	template<Order O, class V=default_grid_val_t<O>>
	requires(is_order_compiled(O))
	struct GridSpan final {
		friend GridSpan<O, const V>;
		friend GridArr<O> grid_arr_copy_from_span<O>(GridConstSpan<O>);
		using T = traits<O>;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;
		constexpr GridSpan(GridArr<O, std::decay_t<V>>& arr) noexcept: cells_(arr.cells_) {}
		constexpr GridSpan(const GridSpan<O, std::remove_const_t<V>>& other) noexcept: cells_(other.cells_) {}
		[[nodiscard]] constexpr V& operator[](const o4x_t coord) const noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr V& at(const o2x_t row, const o2x_t col) const noexcept { return cells_[(T::O2*row)+col]; }
		[[nodiscard]] constexpr std::span<V, T::O2> row_at(const o2x_t i) const noexcept { return static_cast<std::span<V, T::O2>>(cells_.subspan(T::O2*i, T::O2)); }
		[[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
	private:
		std::span<V, T::O4> cells_;
	};


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan<O>) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan<O>) noexcept;


	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_row(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index / (traits<O>::O2)); }
	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_col(const typename traits<O>::o4i_t index) noexcept { return static_cast<traits<O>::o2i_t>(index % (traits<O>::O2)); }
	template<Order O> OKIIDOKU_EXPORT [[nodiscard, gnu::const]] constexpr typename traits<O>::o2i_t rmi_to_box(const typename traits<O>::o2i_t row, const typename traits<O>::o2i_t col) noexcept {
		return static_cast<traits<O>::o2i_t>((row / O) * O) + (col / O);
	}
	// TODO.low consider changing the output to be o2x? then the input would have to be o4x...
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


namespace okiidoku::visitor {

	namespace detail {
		template<class V_=void>
		struct GridSpanAdaptor final {
			template<Order O>
			using V = std::conditional_t<
				std::is_same_v<std::decay_t<V_>, void>, 
				std::conditional_t<
					std::is_const_v<V_>,
					std::add_const_t<default_grid_val_t<O>>,
					default_grid_val_t<O>
				>,
				V_
			>;
			template<Order O>
			using type = mono::GridSpan<O, V>;
		};
		template<class V_=void>
		struct GridArrAdaptor final {
			template<Order O>
			using V = std::conditional_t<
				std::is_same_v<std::decay_t<V_>, void>, 
				default_grid_val_t<O>,
				std::decay_t<V_>
			>;
			template<Order O>
			using type = mono::GridArr<O, V<O>>;
		};
	}

	template<class V>
	struct GridSpan;

	template<class V=void>
	using GridConstSpan = GridSpan<const V>;

	template<class V_=void>
	struct OKIIDOKU_EXPORT GridArr final {
		using V = typename detail::GridArrAdaptor<V_>::V<O>;
		friend GridSpan<V>;
		friend GridConstSpan<V>;
		[[nodiscard]] static GridArr copy_from_span(GridConstSpan<V> src) noexcept;
		[[nodiscard]] constexpr V& operator[](const traits::o4x_t coord) noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr V& at(const traits::o2x_t row, const traits::o2x_t col) noexcept { return cells_[(order_*order_*row)+col]; }
		[[nodiscard]] constexpr const V& operator[](const traits::o4x_t coord) const noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr const V& at(const traits::o2x_t row, const traits::o2x_t col) const noexcept { return cells_[(order_*order_*row)+col]; }
	private:
		Order order_;
		OrderVariantFor<detail::GridArrAdaptor<V_>> cells_;
	};

	template<class V_=void>
	struct OKIIDOKU_EXPORT GridSpan final {
		using V = typename detail::GridSpanAdaptor<V_>::V<O>;
		friend GridSpan<const V>;
		constexpr GridSpan(GridArr<std::decay_t<V>>& arr) noexcept: cells_(arr.cells_) {}
		constexpr GridSpan(const GridSpan<std::remove_const_t<V>>& other) noexcept: cells_(other.cells_) {}
		[[nodiscard]] constexpr V& operator[](const traits::o4x_t coord) const noexcept { return cells_[coord]; }
		[[nodiscard]] constexpr V& at(const traits::o2x_t row, const traits::o2x_t col) const noexcept { return cells_[(order_*order_*row)+col]; }
	private:
		Order order_;
		OrderVariantFor<detail::GridSpanAdaptor<V_>> cells_;
	};


	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan<>) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan<>) noexcept;
}
#endif