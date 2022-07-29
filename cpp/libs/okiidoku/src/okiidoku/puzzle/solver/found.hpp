// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND

#include <okiidoku/o2_bit_arr.hpp>
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
	struct Subset final {
	};

	template<Order O> requires(is_order_compiled(O))
	struct LockedCands final {
		O2BitArr<O> syms;
		int_ts::o3xs_t<O> isec;
		LineType line_type;
		BoxOrLine remove_from_rest_of;
	};

	template<Order O> requires(is_order_compiled(O))
	struct Fish final {
		O2BitArr<O> syms;
		O2BitArr<O> lines;
		LineType elim_from_line_type;
	};


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(std::is_aggregate_v<CellClaimSym<O_>>); \
		static_assert(std::is_aggregate_v<SymClaimCell<O_>>); \
		static_assert(std::is_aggregate_v<Subset<O_>>); \
		static_assert(std::is_aggregate_v<LockedCands<O_>>); \
		static_assert(std::is_aggregate_v<Fish<O_>>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
namespace okiidoku::mono::detail::solver {

	// template<class T>
	// concept Found = requires;
}
#endif