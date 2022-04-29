#ifndef HPP_OKIIDOKU__MORPH__TRANSFORM
#define HPP_OKIIDOKU__MORPH__TRANSFORM

#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>
#include <okiidoku_export.h>

namespace okiidoku::mono::morph {

	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_EXPORT Transformation final {
		using T = traits<O>;
		using mapping_t = T::o2x_smol_t;
		using o1i_t = T::o1i_t;
		using o2i_t = T::o2i_t;

		using label_map_t = std::array<mapping_t, T::O2>;
		using line_map_t = std::array<std::array<mapping_t, T::O1>, T::O1>;

		label_map_t label_map {identity.label_map};
		line_map_t row_map {identity.row_map};
		line_map_t col_map {identity.col_map};
		bool transpose {identity.transpose};

		static constexpr Transformation<O> identity {
			.label_map {[]{ label_map_t _; for (o2i_t i {0}; i < T::O2; ++i) { _[i] = static_cast<mapping_t>(i); } return _; }()},
			.row_map {[]{ line_map_t _; for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<mapping_t>(i); } return _; }()},
			.col_map {[]{ line_map_t _; for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<mapping_t>(i); } return _; }()},
			.transpose {false},
		};

		constexpr bool operator==(const Transformation<O>&) const = default;

		// immediately returns if order is not the same as `src`'s order.
		// changes `dest`'s order to `src`'s order if not already the same.
		void apply_from_to(const Grid<O>& src, Grid<O>& dest) const noexcept;

		// immediately returns if order is not the same as `src`'s order.
		void apply_in_place(Grid<O>&) const noexcept;

		[[nodiscard, gnu::const]] Transformation<O> inverted() const noexcept;
	};
}


namespace okiidoku::visitor::detail::morph {
	struct TransformationAdaptor final {
		static constexpr bool is_ref = false;
		template<Order O>
		using type = mono::morph::Transformation<O>;
	};
}
namespace okiidoku::visitor::morph {

	struct OKIIDOKU_EXPORT Transformation final : public detail::ContainerBase<detail::morph::TransformationAdaptor> {
		using ContainerBase::ContainerBase;

		bool operator==(const Transformation&) const = default;

		// does nothing if any orders are not the same.
		void apply_from_to(const Grid& src, Grid& dest) const noexcept;

		// does nothing if any orders are not the same.
		void apply_in_place(Grid&) const noexcept;

		[[nodiscard, gnu::const]] Transformation inverted() const noexcept;
	};
}
#endif