#ifndef HPP_OKIIDOKU__MORPH__CANON
#define HPP_OKIIDOKU__MORPH__CANON

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

namespace okiidoku::morph {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	template<Order O>
	requires (is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> canonicalize(grid_span_t<O>);

	// contract: T fits traits<O>::o2i_t and canonicalize<O> is compiled. also see canonicalize<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	OKIIDOKU_EXPORT void canonicalize(Order O, std::span<T>);

	extern template void canonicalize<traits<O_MAX>::o2i_smol_t>(Order O, std::span<traits<O_MAX>::o2i_smol_t>);
}
#endif