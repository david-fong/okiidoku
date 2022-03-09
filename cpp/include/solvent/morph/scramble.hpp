#ifndef HPP_SOLVENT__MORPH__SCRAMBLE
#define HPP_SOLVENT__MORPH__SCRAMBLE

#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent_config.hpp"
#include "solvent_export.h"

namespace solvent::morph {

	// must be manually seeded in the main function!
	// Used for scrambling.
	// Note: if parallel access is later required, add a mutex to guard.
	SOLVENT_EXPORT void seed_scrambler_rng(std::uint_fast32_t) noexcept;

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O>
	SOLVENT_EXPORT void scramble(grid_span_t<O>);

	// contract: T fits size<O>::ord2i_t and scramble<O> is compiled. also see scramble<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	SOLVENT_EXPORT void scramble(Order O, std::span<T>);


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void scramble<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif