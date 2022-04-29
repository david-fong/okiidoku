#ifndef HPP_OKIIDOKU__DB__SERDES
#define HPP_OKIIDOKU__DB__SERDES

#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>

#include <iosfwd>

namespace okiidoku::mono::db::serdes {

	constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}

	// contract: the grid is filled.
	template<Order O>
	OKIIDOKU_EXPORT void print_filled(std::ostream& os, const Grid<O>& src);

	// contract: the grid is filled.
	template<Order O>
	OKIIDOKU_EXPORT void parse_filled(std::istream& is, Grid<O>& sink);

	// best used with sparse puzzles.
	template<Order O>
	OKIIDOKU_EXPORT void print_puzzle(std::ostream& os, const Grid<O>& src);

	// best used with sparse puzzles.
	template<Order O>
	OKIIDOKU_EXPORT void parse_puzzle(std::istream& is, Grid<O>& sink);
}


namespace okiidoku::visitor::db::serdes {

	constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}

	// contract: the grid is filled.
	OKIIDOKU_EXPORT void print_filled(std::ostream& os, const Grid& src);

	// contract: the grid is filled.
	OKIIDOKU_EXPORT void parse_filled(std::istream& is, Grid& sink);

	// best used with sparse puzzles.
	OKIIDOKU_EXPORT void print_puzzle(std::ostream& os, const Grid& src);

	// best used with sparse puzzles.
	OKIIDOKU_EXPORT void parse_puzzle(std::istream& is, Grid& sink);
}
#endif