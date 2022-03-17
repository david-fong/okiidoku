#ifndef HPP_SOLVENT__PRINT_2D
#define HPP_SOLVENT__PRINT_2D

#include "solvent/order.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <functional>
#include <span>
#include <cstdint>
#include <cassert>

namespace solvent {

	SOLVENT_EXPORT [[gnu::hot]] void val_to_str(std::ostream&, Order order, uint8_t value) noexcept;

	using print_2d_palette = std::function<void (std::ostream&, uint32_t)>;
	SOLVENT_EXPORT void print_2d(std::ostream&, Order, std::span<const print_2d_palette> grid_views);
}
#endif