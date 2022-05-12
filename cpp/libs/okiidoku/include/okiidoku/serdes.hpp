#ifndef HPP_OKIIDOKU__SERDES
#define HPP_OKIIDOKU__SERDES

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <iosfwd>

/**
These are low-level serialization utilities for portable, efficient data-streaming.
You likely want the provided archive abstraction instead.
But you could use these for direct streaming over a network.
*/
namespace okiidoku::mono {

	// [[nodiscard, gnu::const]] constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
	// 	return (std::bit_width(max_value) + 7) / 8;
	// }

	// contract: the grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void write_solution_grid_to_stream(const Grid<O>&, std::ostream& sink) noexcept;

	// contract: the stream's next bytes contain the result of a call to write_solution_grid_to_stream.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_solution_grid_from_stream(Grid<O>&, std::istream& src) noexcept;

	// contract: the grid is a puzzle with at least one solution.
	// best used with sparse (close to minimal) puzzles.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void print_puzzle_grid_to_stream(const Grid<O>&, std::ostream& sink) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_puzzle_grid_to_stream.
	// post-condition: the grid is a puzzle with at least one solution.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_puzzle_grid_from_stream(Grid<O>&, std::istream& src) noexcept;
}


namespace okiidoku::visitor {

	// contract: the grid is filled and follows the one rule.
	OKIIDOKU_EXPORT void write_solution_grid_to_stream(const Grid&, std::ostream& sink) noexcept;

	// contract: the stream's next bytes contain the result of a call to write_solution_grid_to_stream.
	OKIIDOKU_EXPORT void parse_solution_grid_from_stream(Grid&, std::istream& src) noexcept;

	// contract: the grid is a puzzle with at least one solution.
	// best used with sparse (close to minimal) puzzles.
	OKIIDOKU_EXPORT void print_puzzle_grid_to_stream(const Grid&, std::ostream& sink) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_puzzle_grid_to_stream.
	// post-condition: the grid is a puzzle with at least one solution.
	OKIIDOKU_EXPORT void parse_puzzle_grid_from_stream(Grid&, std::istream& src) noexcept;
}
#endif