#ifndef HPP_OKIIDOKU__VISITOR__MORPH__REL_INFO
#define HPP_OKIIDOKU__VISITOR__MORPH__REL_INFO

// TODO.mid not sure about whether to include this in visitor. not sure why it's even being exported in mono. was probably just in case. No real use-case thought of yet.

#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/compiled_orders.hpp>
#include <okiidoku_export.h>

#include <compare> // strong_ordering, is_eq, etc.

namespace okiidoku::visitor::morph {

	template<Order O>
	using chute_imbalance_t = uint_smolN_t<std::bit_width(2*(O/2)*(O-(O/2)))>;

	struct OKIIDOKU_EXPORT Rel final {
	};

	// contract: the span is a _complete_, valid grid.
	OKIIDOKU_EXPORT grid_arr2d_t<O, Rel> make_rel_table(grid_const_span_t);
}
#endif