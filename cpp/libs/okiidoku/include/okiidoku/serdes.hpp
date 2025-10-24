// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_SERDES
#define HPP_OKIIDOKU_SERDES
#include <okiidoku/detail/export.h>
/** \file
serialization utilities for grids. see also the provided archive abstractions. */

#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

#include <iosfwd>
#include <cstddef>  // size_t

namespace okiidoku {
	using serdes_res_t = std::size_t;
	// \internal ^if there could be IO errors other than EOF, we'd use `std::expected`,
	//  and if it were possible that a grid could encode to zero bytes (it's not), we'd
	//  use `std::optional`. but those aren't the case, so EOF can be `0uz` with `[[nodiscard]]`.
}
namespace okiidoku::mono {
	// TODO Grid member function `write(os)` and free function that returns Grid instead of taking `os`. can make these inline, and non-exported? (to keep symbol table small) (make an \internal note to self about not exporting them if decide to do that.)

	/**
	\pre the grid is filled and follows the one rule.
	\pre `os` is a binary stream- not a text stream.
	\pre `os.good()`.
	\pre `os` will not be used by another thread for the duration of this call.
	\returns the number of bytes written, or `0uz` if there were stream writing errors. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t write_solved(const Grid<O>&, std::ostream& os) noexcept;

	/**
	\pre `is` is a binary stream- not a text stream.
	\pre `is.good()`.
	\pre `is`'s next `byte_count` bytes contain the result of a call to `write_solved`.
	\pre `is` will not be used by another thread for the duration of this call.
	\post parsed grid is filled and follows the one rule.
	\post `is.tellg()` is at end of data, or where EOF occurred.
	\returns `true` if read completed successfully, or `false` on unexpected EOF. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]] bool read_solved(Grid<O>&, std::istream& is, std::size_t byte_count) noexcept;

	/**
	best used with sparse (close to minimal) puzzles.
	\pre the grid follows the one rule.
	\pre `os` is a binary stream- not a text stream.
	\pre `os.good()`.
	\pre `os` will not be used by another thread for the duration of this call.
	\returns the number of bytes written, or `0uz` if there were stream writing errors. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT std::size_t write_puzzle(const Grid<O>&, std::ostream& os) noexcept;

	/**
	\pre `is` is a binary stream- not a text stream.
	\pre `is.good()`.
	\pre `is`'s next bytes contain the result of a call to `write_puzzle`.
	\pre `is` will not be used by another thread for the duration of this call.
	\post the grid follows the one rule.
	\returns `true` if read completed successfully, or `false` on unexpected EOF. */
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[nodiscard]] bool read_puzzle(Grid<O>&, std::istream& is, std::size_t byte_count) noexcept;
}


namespace okiidoku::visitor {

	/** see `okiidoku::mono::write_solved<O>`. */
	OKIIDOKU_EXPORT std::size_t write_solved(const Grid&, std::ostream& os) noexcept;

	/** see `okiidoku::mono::read_solved<O>`. */
	[[nodiscard]] OKIIDOKU_EXPORT bool read_solved(Grid&, std::istream& is, std::size_t byte_count) noexcept;

	/** see `okiidoku::mono::write_puzzle<O>`. */
	OKIIDOKU_EXPORT std::size_t write_puzzle(const Grid&, std::ostream& os) noexcept;

	/** see `okiidoku::mono::read_puzzle<O>`. */
	[[nodiscard]] OKIIDOKU_EXPORT bool read_puzzle(Grid&, std::istream& is, std::size_t byte_count) noexcept;
}
#endif