#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND

#include <okiidoku/house_mask.hpp>
#include <okiidoku/ints.hpp>

#include <type_traits>

namespace okiidoku::mono::detail::solver::found {

	// TODO.mid consider making the constructors private and friending to the corresponding finder. will need to forward declare the finders here.

	template<Order O> requires(is_order_compiled(O))
	struct CellClaimSym final {
		int_ts::o4xs_t<O> rmi;
		int_ts::o2xs_t<O> val;
	};

	template<Order O> requires(is_order_compiled(O))
	struct SymClaimCell final {
		int_ts::o4xs_t<O> rmi;
		int_ts::o2xs_t<O> val;
	};

	template<Order O> requires(is_order_compiled(O))
	struct CellsClaimSyms final {
		HouseMask<O> syms; // what is required
		HouseMask<O> house_cells; // who requires it
		int_ts::o2xs_t<O> house; // where the require-er is
		HouseType house_type; // where the require-er is
	};

	template<Order O> requires(is_order_compiled(O))
	struct SymsClaimCells final {
		HouseMask<O> house_cells; // what is required
		HouseMask<O> syms; // who requires it
		int_ts::o2xs_t<O> house; // where the require-er is
		HouseType house_type; // where the require-er is
	};


	template<Order O> requires(is_order_compiled(O))
	struct LockedCands final {
		int_ts::o2xs_t<O> line;
		int_ts::o1x_t<O> intersecting_box;
		enum Polarity { h, v } line_polarity;
	};


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		static_assert(std::is_aggregate_v<CellClaimSym<O_>>); \
		static_assert(std::is_aggregate_v<SymClaimCell<O_>>); \
		static_assert(std::is_aggregate_v<CellsClaimSyms<O_>>); \
		static_assert(std::is_aggregate_v<SymsClaimCells<O_>>); \
		static_assert(std::is_aggregate_v<LockedCands<O_>>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
namespace okiidoku::mono::detail::solver {

	// template<class T>
	// concept Found = requires;
}
#endif