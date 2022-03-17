#ifndef HPP_SOLVENT__SERDES
#define HPP_SOLVENT__SERDES

#include "solvent/order.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <span>
#include <cstdint>
#include <cassert>

namespace solvent::serdes {

	// contract: grid_view.size() >= O4.
	template<class T>
	SOLVENT_EXPORT void print(std::ostream& os, const Order O, const std::span<T> grid_view) {
		assert(grid_view.size() >= O*O*O*O);
		for (unsigned i {0}; i < O*O*O*O; ++i) {
			os << grid_view[i]; // TODO.high cast is _bad_
		}
	}

	// contract: grid_view.size() >= O4.
	template<class T>
	SOLVENT_EXPORT void parse(std::istream& is, const Order O, const std::span<T> grid_view) {
		assert(grid_view.size() >= O*O*O*O);
		for (unsigned i {0}; i < O*O*O*O; ++i) {
			is >> grid_view[i]; // TODO.high cast is _bad_
		}
	}
}
#endif