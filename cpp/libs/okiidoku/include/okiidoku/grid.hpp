#ifndef HPP_OKIIDOKU__GRID
#define HPP_OKIIDOKU__GRID

#include <okiidoku/traits.hpp>
#include <okiidoku/order_templates.hpp>

#include <ranges>
#include <array>
#include <span>
#include <cassert>


namespace okiidoku::mono {

	namespace detail {
		template<Order O, class V> requires(is_order_compiled(O)) struct GridlikeSpan;
		template<Order O, class V> requires(is_order_compiled(O)) struct GridlikeArr;
	}

	template<Order O> requires(is_order_compiled(O))
	using GridSpan = detail::GridlikeSpan<O, default_grid_val_t<O>>;

	template<Order O> requires(is_order_compiled(O))
	using GridConstSpan = detail::GridlikeSpan<O, const default_grid_val_t<O>>;

	template<Order O> requires(is_order_compiled(O))
	using GridArr = detail::GridlikeArr<O, default_grid_val_t<O>>;

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void copy_grid(GridConstSpan<O> src, GridSpan<O> dest) noexcept;

	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	template<Order O> requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan<O>) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku follows the one rule.
	template<Order O> requires(is_order_compiled(O)) OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan<O>) noexcept;


	namespace detail {
		template<Order O, class V_> requires(is_order_compiled(O))
		class GridlikeArr final { // TODO.mid should this and the span struct be exported? currently all function body definitions are inline so it can be used header-only... but anything not header-only needs to be exported for sure!
		public:
			using val_t = std::decay_t<V_>; // TODO.mid should this instead require std::is_same as decayed?
			friend GridlikeSpan<O, val_t>;
			friend GridlikeSpan<O, const val_t>;
			using T = traits<O>;
			using o2x_t = T::o2x_t;
			using o2i_t = T::o2i_t;
			using o4x_t = T::o4x_t;
			using o4i_t = T::o4i_t;

			[[nodiscard]] constexpr friend auto operator<=>(const GridlikeArr& a, const GridlikeArr& b) noexcept = default;

			// TODO.high consider changing all these inline accessors to take any integral type. should be fine because of the contracts. once done, scan repo for resulting unnecessary static_casts and remove them.
			// contract: coord is in [0, O4). o4i_t only used for convenience of caller.
			[[nodiscard]] constexpr       val_t& operator[](const o4i_t coord)       noexcept { return cells_[coord]; }
			[[nodiscard]] constexpr const val_t& operator[](const o4i_t coord) const noexcept { return cells_[coord]; }

			// contract: row and col are in [0, O2). o2i_t only used for convenience of caller.
			[[nodiscard]] constexpr       val_t& at(const o2i_t row, const o2i_t col)       noexcept { return cells_[(T::O2*row)+col]; }
			[[nodiscard]] constexpr const val_t& at(const o2i_t row, const o2i_t col) const noexcept { return cells_[(T::O2*row)+col]; }

			// contract: row is in [0, O2). o2i_t only used for convenience of caller.
			[[nodiscard]] constexpr std::span<val_t, T::O2> row_at(const o2i_t i) noexcept { return static_cast<std::span<val_t, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }
			// [[nodiscard]] constexpr std::span<const V, T::O2> row_at(const o2i_t i) const noexcept { return static_cast<std::span<V, T::O2>>(std::span(cells_).subspan(T::O2*i, T::O2)); }

			[[nodiscard]] constexpr auto rows() noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
			// [[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
		private:
			std::array<val_t, T::O4> cells_;
		};


		template<Order O, class V=default_grid_val_t<O>> requires(is_order_compiled(O))
		class GridlikeSpan final {
		public:
			using val_t = V;
			friend GridlikeSpan<O, const V>;
			friend void okiidoku::mono::copy_grid<O>(GridConstSpan<O>, GridSpan<O>);
			using T = traits<O>;
			using o2x_t = T::o2x_t;
			using o2i_t = T::o2i_t;
			using o4x_t = T::o4x_t;
			using o4i_t = T::o4i_t;

			explicit constexpr GridlikeSpan(const std::span<V, T::O4> cells) noexcept: cells_(cells) {}

			// create const span from non-const span:
			explicit constexpr GridlikeSpan(const std::span<std::remove_const_t<V>, T::O4> cells) noexcept requires std::is_const_v<V>: cells_(cells) {}

			template<class G, std::enable_if_t<std::is_const_v<V> || (std::is_const_v<G> == std::is_const_v<V>), bool> = true>
			requires std::is_same_v<std::decay_t<G>, GridlikeArr<O, std::decay_t<V>>>
			constexpr GridlikeSpan(G& arr) noexcept: cells_(arr.cells_) {}

			constexpr GridlikeSpan(const GridlikeSpan<O, V>& other) noexcept = default;

			// create const span from non-const span:
			constexpr GridlikeSpan(const GridlikeSpan<O, std::remove_const_t<V>>& other) noexcept requires std::is_const_v<V>: cells_(other.cells_) {}

			// contract: coord is in [0, O4). o4i_t only used for convenience of caller.
			[[nodiscard]] constexpr val_t& operator[](const o4i_t coord) const noexcept { return cells_[coord]; }

			// contract: row and col are in [0, O2). o2i_t only used for convenience of caller.
			[[nodiscard]] constexpr val_t& at(const o2i_t row, const o2i_t col) const noexcept { return cells_[(T::O2*row)+col]; }

			// contract: row is in [0, O2). o2i_t only used for convenience of caller.
			[[nodiscard]] constexpr std::span<val_t, T::O2> row_at(const o2i_t i) const noexcept { return static_cast<std::span<V, T::O2>>(cells_.subspan(T::O2*i, T::O2)); }

			[[nodiscard]] constexpr auto rows() const noexcept { namespace v = std::views; return v::iota(o2i_t{0}, T::O2) | v::transform([&](auto r){ return row_at(r); }); }
		private:
			std::span<val_t, T::O4> cells_;
		};
	};


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


namespace okiidoku::visitor {

	namespace detail {
		template<bool is_const>
		struct GridSpan;
	}

	using GridSpan = detail::GridSpan<false>;
	using GridConstSpan = detail::GridSpan<true>;

	// changes the order of `dest` to that of `src` if they are not already the same.
	OKIIDOKU_EXPORT void copy_grid(GridConstSpan src, GridSpan dest) noexcept;

	// Returns false if any cells in a same house contain the same value.
	// Can be used with incomplete grids.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_follows_rule(GridConstSpan) noexcept;

	// Returns true if none of the cells are empty (equal to O2). Does _not_ check if sudoku is valid.
	OKIIDOKU_EXPORT [[nodiscard]]
	bool grid_is_filled(GridConstSpan) noexcept;

	namespace detail {
		struct GridArrAdaptor final {
			static constexpr bool is_ref = false;
			template<Order O>
			using type = mono::GridArr<O>;
		};
	}

	class GridArr final : public detail::ContainerBase<detail::GridArrAdaptor> {
	public:
		using ContainerBase::ContainerBase;
		using common_val_t = default_grid_val_t;
		// using variant_t = okiidoku::detail::OrderVariantFor<detail::GridArrAdaptor>;
		// friend GridSpan;
		// friend GridConstSpan;

		// contract: coord is in [0, O4).
		[[nodiscard]]       common_val_t& operator[](const traits::o4i_t coord)       noexcept;
		[[nodiscard]] const common_val_t& operator[](const traits::o4i_t coord) const noexcept;

		// contract: row and col are in [0, O2).
		[[nodiscard]]       common_val_t& at(const traits::o2i_t row, const traits::o2i_t col)       noexcept;
		[[nodiscard]] const common_val_t& at(const traits::o2i_t row, const traits::o2i_t col) const noexcept;
	};


	namespace detail {

		template<bool is_const>
		struct GridSpanAdaptor final {
			static constexpr bool is_ref = true;
			template<Order O>
			using type = std::conditional_t<is_const, mono::GridConstSpan<O>, mono::GridSpan<O>>;
		};

		template<bool is_const>
		class GridSpan final : public visitor::detail::ContainerBase<GridSpanAdaptor<is_const>> {
			using variant_t = okiidoku::detail::OrderVariantFor<GridSpanAdaptor<is_const>>;
		public:
			using ContainerBase<GridSpanAdaptor<is_const>>::ContainerBase;
			using common_val_t = std::conditional_t<is_const, const default_grid_val_t, default_grid_val_t>;

			// create const span from non-const span:
			GridSpan(const GridSpan<false>& other) noexcept requires(is_const): GridSpan(static_cast<GridSpan<true>>(other)) {}
			template<Order O> explicit GridSpan(mono::GridSpan<O> mono_span) noexcept requires(is_const): GridSpan(static_cast<mono::GridConstSpan<O>>(mono_span)) {}

			GridSpan(GridArr& arr) noexcept requires(!is_const): GridSpan(std::visit(
				[&]<Order O>(mono::GridArr<O>& mono_arr){ return variant_t(mono::GridSpan<O>(mono_arr)); },
				arr.get_mono_variant()
			)) {}
			GridSpan(const GridArr& arr) noexcept requires(is_const): GridSpan(std::visit(
				[&]<Order O>(const mono::GridArr<O>& mono_arr){ return variant_t(mono::GridConstSpan<O>(mono_arr)); },
				arr.get_mono_variant()
			)) {}
			GridSpan(GridArr& arr) noexcept requires(is_const): GridSpan(std::visit(
				[&]<Order O>(const mono::GridArr<O>& mono_arr){ return variant_t(mono::GridConstSpan<O>(mono_arr)); },
				arr.get_mono_variant()
			)) {}

			// contract: coord is in [0, O4).
			[[nodiscard]] common_val_t& operator[](const traits::o4i_t coord) const noexcept;

			// contract: row and col are in [0, O2).
			[[nodiscard]] common_val_t& at(const traits::o2i_t row, const traits::o2i_t col) const noexcept;
		};
	}
}
#endif