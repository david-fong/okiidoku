#ifndef HPP_OKIIDOKU__MORPH__TRANSFORM
#define HPP_OKIIDOKU__MORPH__TRANSFORM

#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>
#include <okiidoku_export.h>

namespace okiidoku::mono::morph {

	template<Order O>
	requires(is_order_compiled(O))
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
		void apply_from_to(GridConstSpan<O> src, GridSpan<O> dest) const noexcept;
		void apply_in_place(GridSpan<O>) const noexcept;
		Transformation<O> inverted() const noexcept;
	};
}


namespace okiidoku::visitor::morph {

	namespace detail {
		struct TransformationAdaptor final {
			template<Order O>
			using type = mono::morph::Transformation<O>;
		};
	}

	struct OKIIDOKU_EXPORT Transformation final {
		// TODO.high is there a way to make a base class that takes care of the things like the order and
		//  variant fields and making some of the easy/obvious constructors? The order field could really
		//  be a getter that maps the variant index to an order. also see if the monostate can be gotten
		//  rid of... I currently don't have a good reason to be including a monostate option.
		using variant_t = OrderVariantFor<detail::TransformationAdaptor>;

		// uses a std::monostate variant if the specified order is not compiled.
		explicit Transformation(Order O) noexcept;
		template<Order O> constexpr Transformation(mono::Transformation<O> mono_transform): order_{O}, variant_(mono_transform) {}

		constexpr bool operator==(const Transformation&) const = default;
		void apply_from_to(GridConstSpan src, GridSpan dest) const noexcept;
		void apply_in_place(GridSpan) const noexcept;
		Transformation inverted() const noexcept;
	private:
		Order order_;
		variant_t variant_;
	};
}
#endif