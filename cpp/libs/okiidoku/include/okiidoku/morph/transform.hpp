// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_MORPH_TRANSFORM
#define HPP_OKIIDOKU_MORPH_TRANSFORM
#include <okiidoku/detail/export.h>

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/visitor.hpp>
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

#include <array>
#include <type_traits> // is_aggregate_v

namespace okiidoku::mono {

	/**
	\note `post_transpose == pre_transpose + swap(row_map, col_map)`.
	\note `pre_transpose == post_transpose + swap(row_map, col_map)`.
	*/
	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_EXPORT [[gnu::designated_init]] Transformation final {
	private:
		using T = Ints<O>;
		using o1i_t = T::o1i_t;
		using o2i_t = T::o2i_t;
	public:
		using to_t = T::o2xs_t;

		/** legal operations: swap two entries. */
		using sym_map_t = std::array<to_t, T::O2>;

		/** legal operations: swap two entries of the outer layer,
		or of an inner layer (but not between inner layers) */
		using line_map_t = std::array<std::array<to_t, T::O1>, T::O1>;

	public:
		sym_map_t  sym_map {[]noexcept{ sym_map_t  _; for (const auto i : T::O2) { _[i] = i; } return _; }()}; //!< `map[sym_orig] -> sym_new`.
		line_map_t row_map {[]noexcept{ line_map_t _; for (const auto i : T::O2) { _[i/T::O1][i%T::O1] = i; } return _; }()}; //!< `map[chute_orig][chute_cell_orig] -> line_new`.
		line_map_t col_map {[]noexcept{ line_map_t _; for (const auto i : T::O2) { _[i/T::O1][i%T::O1] = i; } return _; }()}; ///< `map[chute_orig][chute_cell_orig] -> line_new`.
		bool post_transpose {false}; ///< whether to transpose after line remapping

		[[nodiscard, gnu::pure]] friend constexpr bool operator==(const Transformation&, const Transformation&) noexcept = default;
		// [[nodiscard, gnu::pure]] friend std::strong_ordering operator<=>(const Transformation&, const Transformation&) noexcept = default;

		/**
		\pre `&src != &dest`.
		\internal I thought about changing this to return dest instead of an outparam,
			which could rely on NRVO, and maybe help the compiler not have to worry that
			dest will be touched by other threads, but I'm not sure how NRVO plays out if
			the caller makes the inparam also the return value sink, and it could be less
			memory-friendly to the Python / JS bindings use-case. */
		void apply_from_to(const Grid<O>& src, Grid<O>& dest) const noexcept;

		void apply_in_place(Grid<O>&) const noexcept;

		[[nodiscard, gnu::pure]] Transformation<O> inverted() const noexcept;
		// TODO: consider aliasing as operator~

		// TODO: for fun: a transformation chaining operation. maybe use || operator? need to investigate commutativity and operator order of evaluation
		// [[nodiscard, gnu::pure]] Transformation<O> chain(const Transformation<O>&) const noexcept;
	};
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(std::is_aggregate_v<Transformation<(O_)>>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor::detail {
	class TransformationAdaptor final {
	public:
		static constexpr bool is_borrow_type = false;
		template<Order O>
		using type = okiidoku::mono::Transformation<O>;
	};
}
namespace okiidoku::visitor {

	struct OKIIDOKU_EXPORT Transformation final : public detail::ContainerBase<detail::TransformationAdaptor> {
		using ContainerBase::ContainerBase;

		[[nodiscard, gnu::pure]] friend bool operator==(const Transformation&, const Transformation&) noexcept = default;

		/** see `okiidoku::mono::apply_from_to`.
		does nothing if the transformation's order is not the same as the source grid's.
		If the dest grid has a different order, it is changed to match the source grid. */
		void apply_from_to(const Grid& src, Grid& dest) const noexcept;

		/** see `okiidoku::mono::apply_in_place`.
		does nothing if the transformation's order is not the same as the grid's. */
		void apply_in_place(Grid&) const noexcept;

		[[nodiscard, gnu::pure]] Transformation inverted() const noexcept;
	};
}
#endif