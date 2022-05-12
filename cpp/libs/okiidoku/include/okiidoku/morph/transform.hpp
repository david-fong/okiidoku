#ifndef HPP_OKIIDOKU__MORPH__TRANSFORM
#define HPP_OKIIDOKU__MORPH__TRANSFORM

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>
#include <okiidoku/detail/export.h>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct OKIIDOKU_EXPORT Transformation final {
		using T = Ints<O>;
		using mapping_t = typename T::o2x_smol_t;
		using o1i_t = typename T::o1i_t;
		using o2i_t = typename T::o2i_t;

		using label_map_t = std::array<mapping_t, T::O2>;
		using  line_map_t = std::array<std::array<mapping_t, T::O1>, T::O1>;

		static constexpr label_map_t identity_label_map {[]{ label_map_t _; for (o2i_t i {0}; i < T::O2; ++i) { _[i] = static_cast<mapping_t>(i); } return _; }()};
		static constexpr  line_map_t identity_row_map   {[]{ line_map_t _;  for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<mapping_t>(i); } return _; }()};
		static constexpr  line_map_t identity_col_map   {[]{ line_map_t _;  for (o2i_t i {0}; i < T::O2; ++i) { _[i/T::O1][i%T::O1] = static_cast<mapping_t>(i); } return _; }()};
		static constexpr        bool identity_post_transpose {false};

		label_map_t label_map {identity_label_map};
		line_map_t row_map {identity_row_map};
		line_map_t col_map {identity_col_map};
		bool post_transpose {identity_post_transpose};

		friend bool operator==(const Transformation&, const Transformation&) noexcept = default;
		// friend std::strong_ordering operator<=>(const Transformation&, const Transformation&) noexcept = default;

		void apply_from_to(const Grid<O>& src, Grid<O>& dest) const noexcept;

		void apply_in_place(Grid<O>&) const noexcept;

		[[nodiscard, gnu::const]] Transformation<O> inverted() const noexcept;
	};
}


namespace okiidoku::visitor::detail {
	struct TransformationAdaptor final {
		static constexpr bool is_borrowtype = false;
		template<Order O>
		using type = okiidoku::mono::Transformation<O>;
	};
}
namespace okiidoku::visitor {

	struct OKIIDOKU_EXPORT Transformation final : public detail::ContainerBase<detail::TransformationAdaptor> {
		using ContainerBase::ContainerBase;

		[[nodiscard]] bool operator==(const Transformation&) const noexcept = default;

		// Does nothing if the transformation's order is not the same as the source grid's.
		// If the dest grid has a different order, it is changed to match the source grid.
		void apply_from_to(const Grid& src, Grid& dest) const noexcept;

		// Does nothing if the transformation's order is not the same as the grid's.
		void apply_in_place(Grid&) const noexcept;

		[[nodiscard, gnu::const]] Transformation inverted() const noexcept;
	};
}
#endif