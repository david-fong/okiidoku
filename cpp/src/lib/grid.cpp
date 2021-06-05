#include "./grid.hpp"

#include <iostream>

namespace solvent::lib {

	template<Order O>
	std::ostream& Grid<O>::print_value(std::ostream& os, const ord2_t value) noexcept {
		static_assert(O <= 6, "I haven't yet decided how to translate for orders > 6.");
		if (value >= O*O) [[unlikely]] {
			return os << ' ';
		} else {
			if constexpr (O < 4) {
				return os << static_cast<unsigned>(value);
			} else if constexpr (O == 5) {
				return os << static_cast<char>('a' +value);
			} else {
				return (value < 10)
					? os << static_cast<unsigned>(value)
					: os << static_cast<char>('a' + value - 10);
			}
		}
	}
}