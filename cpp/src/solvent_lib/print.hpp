#ifndef HPP_SOLVENT_LIB_PRINT
#define HPP_SOLVENT_LIB_PRINT

#include <solvent_lib/size.hpp>
#include <solvent_lib/gen/mod.hpp>

#include <iosfwd>
#include <vector>
#include <functional>

namespace solvent::lib::print {
	//
	using val_grid_t = std::function<uint8_t (uint32_t)>;
	using print_grid_t = std::function<void (std::ostream&, uint32_t)>;

	[[gnu::hot]] void val2str(std::ostream&, Order order, uint8_t value);
	void serial(std::ostream&, Order, val_grid_t grid_view);
	void pretty(std::ostream&, Order, std::vector<print_grid_t> const& grid_views);
}
#endif