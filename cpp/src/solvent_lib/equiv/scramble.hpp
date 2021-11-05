#ifndef HPP_SOLVENT_LIB__EQUIV__SCRAMBLE
#define HPP_SOLVENT_LIB__EQUIV__SCRAMBLE

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
	using vec_grid_t = std::vector<typename size<O>::ord2_t>;

	template<Order O>
	vec_grid_t<O> scramble(vec_grid_t<O> const& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template vec_grid_t<O_> scramble<O_>(vec_grid_t<O_> const&); \
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif