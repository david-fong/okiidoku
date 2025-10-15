// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_SOLVER_FOUND
#define HPP_OKIIDOKU_PUZZLE_SOLVER_FOUND

#include <okiidoku/bit_array.hpp>
#include <okiidoku/ints.hpp>

#include <type_traits>

namespace okiidoku::mono::detail::solver::found {

	// TODO.mid consider making the constructors private and friending to the corresponding finder. will need to forward declare the finders here.

	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] CellClaimSym final {
		Ints<O>::o4xs_t rmi;
		Ints<O>::o2xs_t sym;
	};

	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] SymClaimCell final {
		Ints<O>::o4xs_t rmi;
		Ints<O>::o2xs_t sym;
	};

	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] Subset final {
	};

	/**
	at isec (O1 contiguous cells) `isec` with orientation of `line_type`,
	`syms` can be eliminated from the house of type `remove_from_rest_of`
	containing isec because in the other house type can only (i.e. must)
	have them in that isec. */
	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] LockedCands final {
		O2BitArr<O> syms; // TODO: change this into array<o2i_t, O1>, where items that are O2 are nulls. much better space usage. move field lower
		Ints<O>::o3xs_t isec;
		LineType line_type;
		BoxOrLine remove_from_rest_of;
	};

	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] Fish final {
		O2BitArr<O> syms;
		O2BitArr<O> lines;
		LineType elim_from_line_type;
	};


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(std::is_aggregate_v<CellClaimSym<(O_)>>); \
		static_assert(std::is_aggregate_v<SymClaimCell<(O_)>>); \
		static_assert(std::is_aggregate_v<Subset<(O_)>>); \
		static_assert(std::is_aggregate_v<LockedCands<(O_)>>); \
		static_assert(std::is_aggregate_v<Fish<(O_)>>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
namespace okiidoku::mono::detail::solver {

	// template<class T>
	// concept Found = requires;
}
#endif