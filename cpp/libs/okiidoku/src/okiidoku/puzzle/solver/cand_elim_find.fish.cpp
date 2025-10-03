// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include "okiidoku/puzzle/solver/engine.hpp" // CandsGrid
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/order.hpp>

// #include <algorithm>
// #include <array>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>
namespace okiidoku::mono::detail::solver { template <Order O> requires (is_order_compiled(O)) struct FoundQueues; }

namespace okiidoku::mono::detail::solver { namespace {

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_fish_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		(void)cells_cands; (void)found_queues;
		; // TODO either start from scratch, or try to use the subset finding cache info...
		return false;
	}
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(fish)
	#undef OKIIDOKU_CAND_ELIM_FINDER_DEF

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template UnwindInfo CandElimFind<(O_)>::fish(Engine<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}