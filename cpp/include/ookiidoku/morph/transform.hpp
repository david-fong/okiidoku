#ifndef HPP_OOKIIDOKU__MORPH__TRANSFORM
#define HPP_OOKIIDOKU__MORPH__TRANSFORM

#include <ookiidoku/grid.hpp>
#include <ookiidoku/traits.hpp>
#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

namespace ookiidoku::morph {

	template<Order O>
	requires(is_order_compiled(O))
	struct OOKIIDOKU_EXPORT Transformation final {
		using mapping_t = traits<O>::o2x_smol_t;
		using o1i_t = traits<O>::o1i_t;
		// using o2i_smol_t = traits<O>::o2i_smol_t;
		using o2i_t = traits<O>::o2i_t;

		static constexpr o1i_t O1 = O;
		static constexpr o2i_t O2 = O*O;
		static constexpr typename traits<O>::o4i_t O4 = O*O*O*O;

		using label_map_t = std::array<mapping_t, O2>;
		using line_map_t = std::array<std::array<mapping_t, O1>, O1>;

		label_map_t label_map {identity.label_map};
		line_map_t row_map {identity.row_map};
		line_map_t col_map {identity.col_map};
		bool transpose {identity.transpose};

		static constexpr Transformation<O> identity {
			.label_map {[]{ label_map_t _; for (o2i_t i {0}; i < O2; ++i) { _[i] = static_cast<mapping_t>(i); } return _; }()},
			.row_map {[]{ line_map_t _; for (o2i_t i {0}; i < O2; ++i) { _[i/O1][i%O1] = static_cast<mapping_t>(i); } return _; }()},
			.col_map {[]{ line_map_t _; for (o2i_t i {0}; i < O2; ++i) { _[i/O1][i%O1] = static_cast<mapping_t>(i); } return _; }()},
			.transpose {false},
		};

		bool operator==(const Transformation<O>&) const = default;
		void apply_from_to(grid_const_span_t<O> src, grid_span_t<O> dest) const noexcept;
		void apply_in_place(grid_span_t<O>) const noexcept;
		Transformation<O> inverted() const noexcept;
	};


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		extern template struct Transformation<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}
#endif