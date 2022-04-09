#ifndef HPP_OKIIDOKU__DB__SERDES
#define HPP_OKIIDOKU__DB__SERDES

#include <okiidoku/order.hpp>
#include <okiidoku_export.h>

#include <iosfwd>
#include <span>
#include <bit>

namespace okiidoku::db::serdes {

	// writes the grid contents to the stream using a somewhat compact binary form.
	// multi-byte entries are stored using little-endian.
	// contract: grid_view.size() >= O4.
	template<class V> requires (
		(std::endian::native == std::endian::little) // TODO.wait C++23 for std::byteswap
		&& std::is_integral_v<V>
	)
	OKIIDOKU_EXPORT void print(std::ostream& os, Order O, std::span<const V> grid_view, bool is_grid_filled);

	// reads the grid contents from the stream using the somewhat compact binary form.
	// multi-byte entries are interpreted using little-endian.
	// contract: grid_view.size() >= O4.
	template<class V> requires (
		(std::endian::native == std::endian::little)
		&& std::is_integral_v<V>
		&& (!std::is_const_v<V>)
	)
	OKIIDOKU_EXPORT void parse(std::istream& is, Order O, std::span<V> grid_view, bool is_grid_filled);


	extern template void print<unsigned  char>(std::ostream&, Order, std::span<const unsigned  char>, bool);
	extern template void print<unsigned short>(std::ostream&, Order, std::span<const unsigned short>, bool);
	extern template void parse<unsigned  char>(std::istream&, Order, std::span<unsigned  char>, bool);
	extern template void parse<unsigned short>(std::istream&, Order, std::span<unsigned short>, bool);
}
#endif