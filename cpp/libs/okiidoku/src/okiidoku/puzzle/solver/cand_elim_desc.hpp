#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_DESC
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_DESC

#include <okiidoku/house_mask.hpp>
#include <okiidoku/ints.hpp>

#include <type_traits>

namespace okiidoku::mono::detail::solver::cand_elim_desc {

	// TODO.mid consider making the constructors private and friending to the corresponding finder. will need to forward declare the finders here.

	template<Order O> requires(is_order_compiled(O))
	class CellRequiresSymbol final {
		typename Ints<O>::o4x_smol_t rmi;
		typename Ints<O>::o2x_smol_t val;
	};

	template<Order O> requires(is_order_compiled(O))
	class SymbolRequiresCell final {
		typename Ints<O>::o4x_smol_t rmi;
		typename Ints<O>::o2x_smol_t val;
	};

	template<Order O> requires(is_order_compiled(O))
	class CellsRequireSymbols final {
		HouseMask<O> house_cells;
		typename Ints<O>::o2x_smol_t house;
		HouseType house_type;
	};

	template<Order O> requires(is_order_compiled(O))
	class SymbolsRequireCells final {
		HouseMask<O> syms;
		typename Ints<O>::o2x_smol_t house;
		HouseType house_type;
	};


	template<Order O> requires(is_order_compiled(O))
	class LockedCandidate final {
		typename Ints<O>::o2x_smol_t line;
		typename Ints<O>::o1x_t intersecting_box;
		enum Polarity { h, v } line_polarity;
	};
}
#endif