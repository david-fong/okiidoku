#ifndef HPP_SOLVENT_LIB__MORPH__SCRAMBLE
#define HPP_SOLVENT_LIB__MORPH__SCRAMBLE

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

namespace solvent::lib::morph {

	// must be manually seeded in the main function!
	// Used for scrambling.
	// Note: if parallel access is later required, add a mutex to guard.
	void seed_scrambler_rng(std::uint_fast32_t) noexcept;

	template<Order O>
	void scramble(grid_span_t<O> input);


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void scramble<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif