#ifndef HPP_OKIIDOKU__MORPH__REL_INFO
#define HPP_OKIIDOKU__MORPH__REL_INFO

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <compare> // strong_ordering, is_eq, etc.

namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O))
	struct Rel final {
		using polar_count_lesser_t = detail::uintsN_t<std::bit_width((O*O)/2)>;
		using chute_imbalance_t = detail::uintsN_t<std::bit_width(2*(O/2)*(O-(O/2)))>;

		int_ts::o2is_t<O> count;
		polar_count_lesser_t polar_count_lesser; // smaller value means more imbalance
		chute_imbalance_t chute_imbalance_a;
		chute_imbalance_t chute_imbalance_b;

		[[nodiscard, gnu::pure]] std::strong_ordering operator<=>(const Rel<O>& that) const noexcept {
			// TODO.mid could maybe use std::tie here https://en.cppreference.com/w/cpp/utility/tuple/tie
			#define M_RETURN_IF_NEQ if (std::is_neq(cmp)) [[likely]] { return cmp; }
			std::strong_ordering cmp {that.count <=> count};
			M_RETURN_IF_NEQ; cmp = polar_count_lesser <=> that.polar_count_lesser;
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_a <=> chute_imbalance_a;
			M_RETURN_IF_NEQ; cmp = that.chute_imbalance_b <=> chute_imbalance_b;
			return cmp;
			#undef M_RETURN_IF_NEQ
		}
		[[nodiscard, gnu::pure]] bool operator==(const Rel&) const noexcept = default;
	};

	// contract: the span is a _complete_, valid grid.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] detail::Gridlike<O, Rel<O>> make_rel_table(const Grid<O>&) noexcept;
}
#endif