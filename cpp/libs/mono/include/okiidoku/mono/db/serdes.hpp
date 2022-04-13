#ifndef HPP_OKIIDOKU__MONO__DB__SERDES
#define HPP_OKIIDOKU__MONO__DB__SERDES

#include <okiidoku/mono/grid.hpp>
#include <okiidoku/compiled_orders.hpp>
#include <okiidoku_export.h>

#include <iosfwd>
#include <span>
#include <bit>

namespace okiidoku::mono::db::serdes {

	constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}

	// writes the grid contents to the stream using a somewhat compact binary form.
	template<Order O> requires (
		(std::endian::native == std::endian::little)
	)
	OKIIDOKU_EXPORT void print(std::ostream& os, grid_const_span_t<O> grid_view, bool is_grid_filled);

	// reads the grid contents from the stream using the somewhat compact binary form.
	// multi-byte entries are interpreted using little-endian.
	// contract: grid_view.size() >= O4.
	template<Order O> requires (
		(std::endian::native == std::endian::little)
	)
	OKIIDOKU_EXPORT void parse(std::istream& is, grid_span_t<O> grid_view, bool is_grid_filled);


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template void print<O_>(std::ostream&, grid_const_span_t<O_>, bool); \
		extern template void parse<O_>(std::istream&, grid_span_t<O_>, bool);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O

}
#endif