#ifndef HPP_OKIIDOKU__VISITOR__DB__SERDES
#define HPP_OKIIDOKU__VISITOR__DB__SERDES

#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/mono/db/serdes.hpp>
#include <okiidoku_export.h>

#include <iosfwd>
#include <bit>

namespace okiidoku::visitor::db::serdes {

	// writes the grid contents to the stream using a somewhat compact binary form.
	// multi-byte entries are stored using little-endian.
	requires (std::endian::native == std::endian::little)
	OKIIDOKU_EXPORT void print(std::ostream& os, grid_const_span_t src, bool is_grid_filled);

	// reads the grid contents from the stream using the somewhat compact binary form.
	// multi-byte entries are interpreted using little-endian.
	requires (std::endian::native == std::endian::little)
	OKIIDOKU_EXPORT void parse(std::istream& is, grid_span_t dest, bool is_grid_filled);
}
#endif