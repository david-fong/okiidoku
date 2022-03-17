#include "solvent/serdes.hpp"
#include "solvent_util/str.hpp"

#include <iostream>

namespace solvent::serdes {

	// value must not be greater than O^2.
	void val_to_str(std::ostream& os, const solvent::Order O, const uint8_t value) noexcept {
		if (value == O*O) [[unlikely]] {
			os << ' ';
		} else {
			if (O < 4) {
				os << static_cast<unsigned>(value);
			} else if (O == 5) {
				os << static_cast<char>('a' + value);
			} else {
				(value < 10)
					? os << static_cast<unsigned>(value)
					: os << static_cast<char>('a' + value - 10);
			}
		}
	}
}