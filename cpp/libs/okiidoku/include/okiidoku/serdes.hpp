// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__SERDES
#define HPP_OKIIDOKU__SERDES

#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

#include <iosfwd>

/**
these are low-level serialization utilities for portable, efficient data-streaming.
you likely want the provided archive abstraction instead.
but you could use these for direct streaming over a network.
*/
namespace okiidoku::mono {

	// [[nodiscard, gnu::const]] constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
	// 	return (std::bit_width(max_value) + 7) / 8;
	// }

	// TODO efficient mechanism to opt-in/out of error-checking in the parsing functions.

	/**
	\pre the grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void write_solution_grid_to_stream(const Grid<O>&, std::ostream& sink) noexcept;

	/**
	\pre the stream's next bytes contain the result of a call to `write_solution_grid_to_stream`.
	\post parsed grid is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_solution_grid_from_stream(Grid<O>&, std::istream& src) noexcept;

	/**
	best used with sparse (close to minimal) puzzles.
	\pre the grid follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void print_puzzle_grid_to_stream(const Grid<O>&, std::ostream& sink) noexcept;

	/**
	\pre the stream's next bytes contain the result of a call to `print_puzzle_grid_to_stream`.
	\post the grid follows the one rule.. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_puzzle_grid_from_stream(Grid<O>&, std::istream& src) noexcept;
}


namespace okiidoku::visitor {

	/** see `write_solution_grid_to_stream`. */
	OKIIDOKU_EXPORT void write_solution_grid_to_stream(const Grid&, std::ostream& sink) noexcept;

	/** see `parse_solution_grid_from_stream`. */
	OKIIDOKU_EXPORT void parse_solution_grid_from_stream(Grid&, std::istream& src) noexcept;

	/** see `print_puzzle_grid_to_stream`. */
	OKIIDOKU_EXPORT void print_puzzle_grid_to_stream(const Grid&, std::ostream& sink) noexcept;

	/** see `parse_puzzle_grid_from_stream`. */
	OKIIDOKU_EXPORT void parse_puzzle_grid_from_stream(Grid&, std::istream& src) noexcept;
}
#endif