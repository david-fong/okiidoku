#ifndef HPP_SOLVENT__PRINT_2D
#define HPP_SOLVENT__PRINT_2D

#include "solvent/size.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <functional>
#include <span>

namespace solvent {

	// SOLVENT_EXPORT [[gnu::hot]] void val_to_str(std::ostream&, Order order, uint8_t value) noexcept;

	using print_2d_palette = std::function<size<O_MAX>::ord2i_least_t (size<O_MAX>::ord4x_t coord)>;
	SOLVENT_EXPORT void print_2d(std::ostream&, Order, std::span<const print_2d_palette> grid_views);
}
#endif