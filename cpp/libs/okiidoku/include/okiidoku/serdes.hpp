// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_SERDES
#define HPP_OKIIDOKU_SERDES

#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

#include <iosfwd>
#include <cstddef> // size_t

/**
these are low-level serialization utilities for portable, efficient data-streaming.
you likely want the provided archive abstraction instead.
but you could use these for direct streaming over a network.
*/
namespace okiidoku::mono {

	// [[nodiscard, gnu::const]] constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
	// 	return (std::bit_width(max_value) + (CHAR_BIT-1)) / CHAR_BIT;
	// }

	// TODO efficient mechanism to opt-in/out of error-checking in the parsing functions.
	// TODO Grid member function `write(os)` and free function that returns Grid instead of taking `sink`. can make these inline, and non-exported? (to keep symbol table small) (make an \internal note to self about not exporting them if decide to do that.)

	/**
	\pre the stream is a binary stream- not a text stream.
	\pre the grid is filled and follows the one rule.
	\returns the number of bytes written. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t write_solved(const Grid<O>&, std::ostream& sink) noexcept;

	/**
	\pre the stream is a binary stream- not a text stream.
	\pre the stream's next bytes contain the result of a call to `write_solved`.
	\post parsed grid is filled and follows the one rule.
	\returns the number of bytes read. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t read_solved(Grid<O>&, std::istream& src) noexcept;

	/**
	best used with sparse (close to minimal) puzzles.
	\pre the stream is a binary stream- not a text stream.
	\pre the grid follows the one rule.
	\returns the number of bytes written. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t write_puzzle(const Grid<O>&, std::ostream& sink) noexcept;

	/**
	\pre the stream is a binary stream- not a text stream.
	\pre the stream's next bytes contain the result of a call to `write_puzzle`.
	\post the grid follows the one rule.
	\returns the number of bytes read. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t read_puzzle(Grid<O>&, std::istream& src) noexcept;
}


namespace okiidoku::visitor {

	/** see `write_solved`. */
	OKIIDOKU_EXPORT std::size_t write_solved(const Grid&, std::ostream& sink) noexcept;

	/** see `read_solved`. */
	OKIIDOKU_EXPORT std::size_t read_solved(Grid&, std::istream& src) noexcept;

	/** see `write_puzzle`. */
	OKIIDOKU_EXPORT std::size_t write_puzzle(const Grid&, std::ostream& sink) noexcept;

	/** see `read_puzzle`. */
	OKIIDOKU_EXPORT std::size_t read_puzzle(Grid&, std::istream& src) noexcept;
}
#endif