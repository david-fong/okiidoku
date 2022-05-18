#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_DESC
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_DESC

#include <okiidoku/house_mask.hpp>
#include <okiidoku/ints.hpp>

#include <type_traits>

namespace okiidoku::mono::detail::solver::cand_elim_desc {

	// TODO.mid consider making the constructors private and friending to the corresponding finder. will need to forward declare the finders here.

	template<Order O> requires(is_order_compiled(O))
	struct CellRequiresSymbol final {
		typename Ints<O>::o4x_smol_t rmi;
		typename Ints<O>::o2x_smol_t val;
	};

	template<Order O> requires(is_order_compiled(O))
	struct SymbolRequiresCell final {
		typename Ints<O>::o4x_smol_t rmi;
		typename Ints<O>::o2x_smol_t val;
	};

	template<Order O> requires(is_order_compiled(O))
	struct CellsRequireSymbols final {
		HouseMask<O> syms; // what is required
		HouseMask<O> house_cells; // who requires it
		typename Ints<O>::o2x_smol_t house; // where the require-er is
		HouseType house_type; // where the require-er is
	};

	template<Order O> requires(is_order_compiled(O))
	struct SymbolsRequireCells final {
		HouseMask<O> house_cells; // what is required
		HouseMask<O> syms; // who requires it
		typename Ints<O>::o2x_smol_t house; // where the require-er is
		HouseType house_type; // where the require-er is
	};


	template<Order O> requires(is_order_compiled(O))
	struct LockedCandidate final {
		typename Ints<O>::o2x_smol_t line;
		typename Ints<O>::o1x_t intersecting_box;
		enum Polarity { h, v } line_polarity;
	};


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		static_assert(std::is_aggregate_v<CellRequiresSymbol<O_>>); \
		static_assert(std::is_aggregate_v<SymbolRequiresCell<O_>>); \
		static_assert(std::is_aggregate_v<CellsRequireSymbols<O_>>); \
		static_assert(std::is_aggregate_v<SymbolsRequireCells<O_>>); \
		static_assert(std::is_aggregate_v<LockedCandidate<O_>>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif