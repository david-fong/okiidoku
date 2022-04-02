#include <okiidoku/db/serdes.hpp>

#include <iostream>
#include <cassert>

namespace okiidoku::db::serdes {

	template<class T> requires (
		(std::endian::native == std::endian::little)
		&& std::is_integral_v<T>
	)
	OKIIDOKU_EXPORT void print(std::ostream& os, const Order O, const std::span<const T> grid_view) {
		const Order O4 = O*O*O*O;
		assert(grid_view.size() >= O4);
		const std::streamsize val_size = get_min_bytes_to_store(O*O);
		if (sizeof(T) == val_size) {
			os.write(reinterpret_cast<const char*>(grid_view.data()), val_size * O4);
		} else {
			for (unsigned i {0}; i < O4; ++i) {
				os.write(reinterpret_cast<const char*>(&grid_view[i]), val_size);
			}
		}
	}


	template<class T> requires (
		(std::endian::native == std::endian::little)
		&& std::is_integral_v<T>
		&& (!std::is_const_v<T>)
	)
	OKIIDOKU_EXPORT void parse(std::istream& is, const Order O, const std::span<T> grid_view) {
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
	}


	template void print<unsigned  char>(std::ostream& os, Order O, std::span<const unsigned  char> grid_view);
	template void print<unsigned short>(std::ostream& os, Order O, std::span<const unsigned short> grid_view);
	template void parse<unsigned  char>(std::istream& is, Order O, std::span<unsigned  char> grid_view);
	template void parse<unsigned short>(std::istream& is, Order O, std::span<unsigned short> grid_view);
}