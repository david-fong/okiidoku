// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/detail/pgo_use_check_needs_rebuild.hpp>
#include <okiidoku/puzzle2/solver/cands.hpp>

namespace okiidoku::mono::detail::solver2 {


	template<Order O> requires(is_order_compiled(O) && (O < order_threshold_to_use_compact_cands))
	void CandsPov final {
	}
}