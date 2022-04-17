#include <okiidoku/db/serdes.hpp>

#include <iostream>
#include <bitset>
#include <cassert>

namespace okiidoku::mono::db::serdes {

	template<Order O> requires (
		(std::endian::native == std::endian::little)
	)
	OKIIDOKU_EXPORT void print(std::ostream& os, const GridConstSpan_t<O> grid_view, const bool is_grid_filled) {
		// const Order O2 = O*O;
		const Order O4 = traits<O>::O4;
		if (is_grid_filled) {
			// assert(is_grid_filled<O>(grid)); // TODO.high
		}
		constexpr std::streamsize val_size = get_min_bytes_to_store(O*O);
		os.write(reinterpret_cast<const char*>(grid_view.data()), val_size * O4);
		(void)is_grid_filled;
		// for (unsigned row {0}; row < O4; ++row) {
		// 	std::bitset<> buf;
		// 	for (unsigned col {0}; col < O2; ++col) {
		// 		os.write(reinterpret_cast<const char*>(&grid_view[row]), val_size);
		// 	}
		// }
	}


	template<Order O> requires (
		(std::endian::native == std::endian::little)
	)
	OKIIDOKU_EXPORT void parse(std::istream& is, const GridSpan<O> grid_view, const bool is_grid_filled) {
		// const Order O2 = traits<O>::O2;
		const Order O4 = traits<O>::O4;
		constexpr std::streamsize val_size = get_min_bytes_to_store(O*O);
		is.read(reinterpret_cast<char*>(grid_view.data()), sizeof(val_size) * O4);
		(void)is_grid_filled;
		// for (unsigned row {0}; row < O2; ++row) {
		// 	for (unsigned col {0}; col < O2; ++col) {
		// 		is.read(reinterpret_cast<char*>(&grid_view[row]), sizeof(T));
		// 	}
		// }
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void print<O_>(std::ostream&, GridConstSpan_t<O_>, bool); \
		template void parse<O_>(std::istream&, GridSpan<O_>, bool);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}