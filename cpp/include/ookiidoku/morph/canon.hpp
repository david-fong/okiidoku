#ifndef HPP_OOKIIDOKU__MORPH__CANON
#define HPP_OOKIIDOKU__MORPH__CANON

#include <ookiidoku/morph/transform.hpp>
#include <ookiidoku/grid.hpp>
#include <ookiidoku/size.hpp>
#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

namespace ookiidoku::morph {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	template<Order O>
	requires (is_order_compiled(O))
	OOKIIDOKU_EXPORT Transformation<O> canonicalize(grid_span_t<O>);

	// contract: T fits size<O>::ord2i_t and canonicalize<O> is compiled. also see canonicalize<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	OOKIIDOKU_EXPORT void canonicalize(Order O, std::span<T>);

	extern template void canonicalize<size<O_MAX>::ord2i_least_t>(Order O, std::span<size<O_MAX>::ord2i_least_t>);
}
#endif