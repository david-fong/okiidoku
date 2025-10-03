// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__CANON_STAGES
#define HPP_OKIIDOKU__DETAIL__CANON_STAGES

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

namespace okiidoku::mono::detail {

	/** implemented in canon_sym.cpp.
	\pre the grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	typename Transformation<O>::sym_map_t canon_sym(Grid<O>&) noexcept;

	/** implemented in canon_pos.cpp.
	\pre the grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canon_pos(Grid<O>&) noexcept;
}
#endif