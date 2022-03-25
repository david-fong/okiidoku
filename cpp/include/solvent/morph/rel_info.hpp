#ifndef HPP_SOLVENT__MORPH__REL_INFO
#define HPP_SOLVENT__MORPH__REL_INFO

#include "solvent/grid.hpp"
#include "solvent/order.hpp"
#include "solvent/solvent_config.hpp"
#include "solvent_export.h"

#include <compare>   // strong_ordering, is_eq, etc.

namespace solvent::morph {

	template<Order O>
	using chute_imbalance_t = uint_leastN_t<std::bit_width(2*(O/2)*(O-(O/2)))>;

	template<Order O>
	requires (is_order_compiled(O))
	struct SOLVENT_EXPORT Rel final {
		using polar_count_lesser_t = uint_leastN_t<std::bit_width((O*O)/2)>;
		typename size<O>::ord2i_least_t count;
		polar_count_lesser_t polar_count_lesser; // smaller value means more imbalance
		chute_imbalance_t<O> chute_imbalance_a;
		chute_imbalance_t<O> chute_imbalance_b;

		std::strong_ordering operator<=>(const Rel<O>& that) const {
			#define M_RETURN_IF_NEQ if (std::is_neq(cmp)) [[likely]] { return cmp; }
			std::strong_ordering cmp = that.count <=> count;
			M_RETURN_IF_NEQ; cmp = polar_count_lesser <=> that.polar_count_lesser;
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_a <=> chute_imbalance_a; // TODO.try could these be omitted from use above a certain order?
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_b <=> chute_imbalance_b;
			return cmp;
		}
	};

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O>
	requires (is_order_compiled(O))
	SOLVENT_EXPORT grid_arr_t<O, Rel<O>> make_rel_table(grid_const_span_t<O>);


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template struct Rel<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif