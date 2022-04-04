#ifndef HPP_OKIIDOKU__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MORPH__SCRAMBLE

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

namespace okiidoku::morph {

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O>
	requires (is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> scramble(grid_span_t<O>);

	// contract: T fits traits<O>::o2i_t and scramble<O> is compiled. also see scramble<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	OKIIDOKU_EXPORT void scramble(Order O, std::span<T>);
}
#endif