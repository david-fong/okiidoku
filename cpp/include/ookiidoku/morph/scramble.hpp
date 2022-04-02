#ifndef HPP_OOKIIDOKU__MORPH__SCRAMBLE
#define HPP_OOKIIDOKU__MORPH__SCRAMBLE

#include <ookiidoku/grid.hpp>
#include <ookiidoku/traits.hpp>
#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

namespace ookiidoku::morph {

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O>
	requires (is_order_compiled(O))
	OOKIIDOKU_EXPORT void scramble(grid_span_t<O>);

	// contract: T fits traits<O>::o2i_t and scramble<O> is compiled. also see scramble<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	OOKIIDOKU_EXPORT void scramble(Order O, std::span<T>);
}
#endif