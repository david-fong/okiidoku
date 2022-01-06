#ifndef HPP_SOLVENT_LIB__EQUIV__ANALYZE
#define HPP_SOLVENT_LIB__EQUIV__ANALYZE

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

namespace solvent::lib::equiv {

	template<Order O>
	struct GridStats {
		struct LabelStats {
		};
		LabelStats labels;
		// blocks;
	};

	/** */
	template<Order O> [[nodiscard]]
	[[nodiscard, gnu::const]] GridStats<O> analyze(const grid_vec_t<O>& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template GridStats<O_> analyze<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif