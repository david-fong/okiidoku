#ifndef HPP_SOLVENT__PRINT
#define HPP_SOLVENT__PRINT

#include "solvent/order.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <functional>
#include <span>
#include <cstdint>

namespace solvent::print {

	SOLVENT_EXPORT [[gnu::hot]] void val_to_str(std::ostream&, Order order, uint8_t value);

	SOLVENT_EXPORT void text(std::ostream&, Order, std::span<const std::uint8_t> grid_view);

	using print_grid_t = std::function<void (std::ostream&, uint32_t)>;
	SOLVENT_EXPORT void pretty(std::ostream&, Order, std::span<const print_grid_t> grid_views);
}
#endif