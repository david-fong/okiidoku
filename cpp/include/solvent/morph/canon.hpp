#ifndef HPP_SOLVENT__MORPH__CANON
#define HPP_SOLVENT__MORPH__CANON

#include "solvent/morph/transform.hpp"
#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent/solvent_config.hpp"
#include "solvent_export.h"

namespace solvent::morph {

	// contract: the span is a _complete_, valid grid.
	template<Order O>
	requires (is_order_compiled(O))
	SOLVENT_EXPORT Transformation<O> canonicalize(grid_span_t<O>);

	// contract: T fits size<O>::ord2i_t and canonicalize<O> is compiled. also see canonicalize<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	SOLVENT_EXPORT void canonicalize(Order O, std::span<T>);

	extern template void canonicalize<size<O_MAX>::ord2i_least_t>(Order O, std::span<size<O_MAX>::ord2i_least_t>);
}
#endif