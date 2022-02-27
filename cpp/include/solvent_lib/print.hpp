#ifndef HPP_SOLVENT_LIB__PRINT
#define HPP_SOLVENT_LIB__PRINT

#include <solvent_lib/order.hpp>

#include <iosfwd>
#include <functional>
#include <span>
#include <cstdint>

namespace solvent::lib::print {

	[[gnu::hot]] void val2str(std::ostream&, Order order, uint8_t value);

	using val_grid_t = std::function<uint8_t (uint32_t)>;
	void text(std::ostream&, Order, val_grid_t grid_view);
	void text(std::ostream&, Order, std::span<const std::uint8_t> grid_view);

	using print_grid_t = std::function<void (std::ostream&, uint32_t)>;
	void pretty(std::ostream&, Order, std::span<const print_grid_t> grid_views);
}
#endif