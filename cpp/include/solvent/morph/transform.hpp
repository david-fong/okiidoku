#ifndef HPP_SOLVENT__MORPH__TRANSFORM
#define HPP_SOLVENT__MORPH__TRANSFORM

#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent/solvent_config.hpp"
#include "solvent_export.h"

namespace solvent::morph {

	template<Order O>
	requires(is_order_compiled(O))
	struct SOLVENT_EXPORT Transformation final {
		using mapping_t = size<O>::ord2x_least_t;
		using ord1i_t = size<O>::ord1i_t;
		// using ord2i_least_t = size<O>::ord2i_least_t;
		using ord2i_t = size<O>::ord2i_t;

		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr typename size<O>::ord4i_t O4 = O*O*O*O;

		using label_map_t = std::array<mapping_t, O2>;
		using line_map_t = std::array<std::array<mapping_t, O1>, O1>;

		label_map_t label_map {[]{ label_map_t _; for (ord2i_t i {0}; i < O2; ++i) { _[i] = static_cast<mapping_t>(i); } return _; }()};
		bool transpose {false};
		line_map_t row_map {[]{ line_map_t _; for (ord2i_t i {0}; i < O2; ++i) { _[i/O1][i%O1] = static_cast<mapping_t>(i); } return _; }()};
		line_map_t col_map {[]{ line_map_t _; for (ord2i_t i {0}; i < O2; ++i) { _[i/O1][i%O1] = static_cast<mapping_t>(i); } return _; }()};

		void apply_to(grid_span_t<O>) const noexcept;
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template struct Transformation<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif