// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__TRANSFORM
#define HPP_OKIIDOKU__MORPH__TRANSFORM

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_EXPORT [[gnu::designated_init]] Transformation final {
	private:
		using T = Ints<O>;
		using o1i_t = int_ts::o1i_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
	public:
		using to_t = int_ts::o2xs_t<O>;

		/** legal operations: swap two entries. */
		using sym_map_t = std::array<to_t, T::O2>;

		/** legal operations: swap two entries of the outer layer,
		or of an inner layer (but not between inner layers) */
		using line_map_t = std::array<std::array<to_t, T::O1>, T::O1>;

		static const Transformation identity;

	public:
		sym_map_t sym_map {identity.sym_map}; //!< `map[label_orig] -> label_new`.
		line_map_t row_map {identity.row_map}; //!< `map[chute_orig][chute_cell_orig] -> line_new`.
		line_map_t col_map {identity.col_map}; ///< `map[chute_orig][chute_cell_orig] -> line_new`.
		bool post_transpose {identity.post_transpose}; ///< whether to transpose after line remapping

		[[nodiscard, gnu::pure]] friend bool operator==(const Transformation&, const Transformation&) noexcept = default;
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

		// TODO: for fun: a transformation chaining operation
		// [[nodiscard, gnu::pure]] Transformation<O> chain(const Transformation<O>&) const noexcept;
	};

	template<Order O> requires(is_order_compiled(O))
	constexpr Transformation<O> Transformation<O>::identity {
		.sym_map {[]{ sym_map_t _{}; for (o2i_t i {0}; i < T::O2; ++i) { _[i] = static_cast<to_t>(i); } return _; }()},
		.row_map {[]{ line_map_t _{};  for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<to_t>(i); } return _; }()},
		.col_map {[]{ line_map_t _{};  for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<to_t>(i); } return _; }()},
		.post_transpose {false},
	};
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

		/**
		\copydoc okiidoku::mono::apply_from_to
		Does nothing if the transformation's order is not the same as the source grid's.
		If the dest grid has a different order, it is changed to match the source grid. */
		void apply_from_to(const Grid& src, Grid& dest) const noexcept;

		/**
		\copydoc okiidoku::mono::apply_in_place
		Does nothing if the transformation's order is not the same as the grid's. */
		void apply_in_place(Grid&) const noexcept;

		[[nodiscard, gnu::pure]] Transformation inverted() const noexcept;
	};
}
#endif