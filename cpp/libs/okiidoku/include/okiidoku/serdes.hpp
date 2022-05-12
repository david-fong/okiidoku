#ifndef HPP_OKIIDOKU__SERDES
#define HPP_OKIIDOKU__SERDES

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <iosfwd>

/**
These are low-level serialization utilities. You likely want the
provided database abstraction instead. These utilities are probably
only useful to you if you want to do data-streaming over a network.
*/
namespace okiidoku::mono::serdes {

	[[nodiscard, gnu::const]] constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}

	// contract: the grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void print_filled(std::ostream& sink, const Grid<O>& src) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_filled.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_filled(std::istream& src, Grid<O>& sink) noexcept;

	// best used with sparse puzzles.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void print_puzzle(std::ostream& sink, const Grid<O>& src) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_puzzle.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void parse_puzzle(std::istream& src, Grid<O>& sink) noexcept;
}


namespace okiidoku::visitor::serdes {

	// contract: the grid is filled and follows the one rule.
	OKIIDOKU_EXPORT void print_filled(std::ostream& sink, const Grid& src) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_filled.
	OKIIDOKU_EXPORT void parse_filled(std::istream& src, Grid& sink) noexcept;

	// best used with sparse puzzles.
	OKIIDOKU_EXPORT void print_puzzle(std::ostream& sink, const Grid& src) noexcept;

	// contract: the stream's next bytes contain the result of a call to print_puzzle.
	OKIIDOKU_EXPORT void parse_puzzle(std::istream& src, Grid& sink) noexcept;
}
#endif