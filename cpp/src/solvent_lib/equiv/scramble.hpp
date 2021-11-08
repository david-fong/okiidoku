#ifndef HPP_SOLVENT_LIB__EQUIV__SCRAMBLE
#define HPP_SOLVENT_LIB__EQUIV__SCRAMBLE

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <random>
#include <vector>
#include <array>
#include <numeric>   // iota,

namespace solvent::lib::equiv {

	// must be manually seeded in the main function!
	// Used for scrambling.
	// NOTE: if parallel access is later required, add a mutex to guard.
	extern std::mt19937 ScramblerRng;

	template<Order O>
	grid_vec_t<O> scramble(grid_vec_t<O> const& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> scramble<O_>(grid_vec_t<O_> const&); \
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif