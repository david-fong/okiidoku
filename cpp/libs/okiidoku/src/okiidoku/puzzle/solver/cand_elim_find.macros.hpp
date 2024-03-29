// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND__MACROS
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND__MACROS

// yes, I know macros are generally recommended against.
// yes, I know macros do not get scoped to namespaces.
// I am not doing this because I like macros.
namespace okiidoku::mono::detail::solver {

	#define OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS \
		OKIIDOKU_MONO_INT_TS_TYPEDEFS \
		using rmi_t [[maybe_unused]] = int_ts::o4xs_t<O>;


	#define OKIIDOKU_CAND_ELIM_FINDER_DEF(TECHNIQUE_NAME) \
	template<Order O> requires(is_order_compiled(O)) \
	UnwindInfo CandElimFind<O>::TECHNIQUE_NAME(Engine<O>& engine) noexcept { \
		OKIIDOKU_CONTRACT_ASSERT(!engine.no_more_solns()); \
		if (engine.get_num_unsolved() == 0) [[unlikely]] { return UnwindInfo::make_no_unwind(); } \
		const auto needs_unwind {find_ ## TECHNIQUE_NAME ## _and_check_needs_unwind(engine.cells_cands(), engine.get_found_queues_())}; \
		if (needs_unwind) { return engine.unwind_one_stack_frame(); } \
		return UnwindInfo::make_no_unwind(); \
	}
}
#endif