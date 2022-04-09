#include <okiidoku/db/serdes.hpp>

#include <iostream>
#include <bitset>
#include <cassert>

namespace okiidoku::db::serdes {

	template<class T> requires (
		(std::endian::native == std::endian::little)
		&& std::is_integral_v<T>
	)
	OKIIDOKU_EXPORT void print(std::ostream& os, const Order O, const std::span<const T> grid_view, const bool is_grid_filled) {
		// const Order O2 = O*O;
		const Order O4 = O*O*O*O;
		assert(grid_view.size() >= O4);
		if (is_grid_filled) {
			// assert(is_grid_filled<O>(grid)); // TODO.high
		}
		const std::streamsize val_size = get_min_bytes_to_store(O*O);
		if (sizeof(T) == val_size) {
			os.write(reinterpret_cast<const char*>(grid_view.data()), val_size * O4);
		} else {
			for (unsigned i {0}; i < O4; ++i) {
				os.write(reinterpret_cast<const char*>(&grid_view[i]), val_size);
			}
		}
		(void)is_grid_filled;
		// for (unsigned row {0}; row < O4; ++row) {
		// 	std::bitset<> buf;
		// 	for (unsigned col {0}; col < O2; ++col) {
		// 		os.write(reinterpret_cast<const char*>(&grid_view[row]), val_size);
		// 	}
		// }
	}


	template<class T> requires (
		(std::endian::native == std::endian::little)
		&& std::is_integral_v<T>
		&& (!std::is_const_v<T>)
	)
	OKIIDOKU_EXPORT void parse(std::istream& is, const Order O, const std::span<T> grid_view, const bool is_grid_filled) {
		// const Order O2 = O*O;
		const Order O4 = O*O*O*O;
		assert(grid_view.size() >= O4);
		const std::streamsize val_size = get_min_bytes_to_store(O*O);
		if (sizeof(T) == val_size) {
			is.read(reinterpret_cast<char*>(grid_view.data()), sizeof(T) * O4);
		} else {
			for (unsigned i {0}; i < O4; ++i) {
				is.read(reinterpret_cast<char*>(&grid_view[i]), sizeof(T));
			}
		}
		(void)is_grid_filled;
		// for (unsigned row {0}; row < O2; ++row) {
		// 	for (unsigned col {0}; col < O2; ++col) {
		// 		is.read(reinterpret_cast<char*>(&grid_view[row]), sizeof(T));
		// 	}
		// }
	}


	template void print<unsigned  char>(std::ostream&, Order, std::span<const unsigned  char>, bool);
	template void print<unsigned short>(std::ostream&, Order, std::span<const unsigned short>, bool);
	template void parse<unsigned  char>(std::istream&, Order, std::span<unsigned  char>, bool);
	template void parse<unsigned short>(std::istream&, Order, std::span<unsigned short>, bool);
}