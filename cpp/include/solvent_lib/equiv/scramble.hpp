#ifndef HPP_SOLVENT_LIB__EQUIV__SCRAMBLE
#define HPP_SOLVENT_LIB__EQUIV__SCRAMBLE

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

namespace solvent::lib::equiv {

	// must be manually seeded in the main function!
	// Used for scrambling.
	// NOTE: if parallel access is later required, add a mutex to guard.
	void seed_scrambler_rng(std::uint_fast32_t) noexcept;

	template<Order O> [[nodiscard]]
	grid_vec_t<O> scramble(const grid_vec_t<O>& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> scramble<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif