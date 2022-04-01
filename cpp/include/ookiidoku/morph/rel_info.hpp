#ifndef HPP_OOKIIDOKU__MORPH__REL_INFO
#define HPP_OOKIIDOKU__MORPH__REL_INFO

#include <ookiidoku/grid.hpp>
#include <ookiidoku/order.hpp>
#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

#include <compare>   // strong_ordering, is_eq, etc.

namespace ookiidoku::morph {

	template<Order O>
	using chute_imbalance_t = uint_leastN_t<std::bit_width(2*(O/2)*(O-(O/2)))>;

	template<Order O>
	requires (is_order_compiled(O))
	struct OOKIIDOKU_EXPORT Rel final {
		using polar_count_lesser_t = uint_leastN_t<std::bit_width((O*O)/2)>;
		typename size<O>::ord2i_least_t count;
		polar_count_lesser_t polar_count_lesser; // smaller value means more imbalance
		chute_imbalance_t<O> chute_imbalance_a;
		chute_imbalance_t<O> chute_imbalance_b;

		std::strong_ordering operator<=>(const Rel<O>& that) const {
			// TODO.mid could maybe use std::tie here https://en.cppreference.com/w/cpp/utility/tuple/tie
			#define M_RETURN_IF_NEQ if (std::is_neq(cmp)) [[likely]] { return cmp; }
			std::strong_ordering cmp = that.count <=> count;
			M_RETURN_IF_NEQ; cmp = polar_count_lesser <=> that.polar_count_lesser;
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_a <=> chute_imbalance_a;
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_b <=> chute_imbalance_b;
			return cmp;
		}
	};

	// contract: the span is a _complete_, valid grid.
	template<Order O>
	requires (is_order_compiled(O))
	OOKIIDOKU_EXPORT grid_arr2d_t<O, Rel<O>> make_rel_table(grid_const_span_t<O>);


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		extern template struct Rel<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}
#endif