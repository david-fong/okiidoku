#ifndef HPP_SOLVENT_LIB_PRINT
#define HPP_SOLVENT_LIB_PRINT

#include <solvent_lib/size.hpp>
#include <solvent_lib/gen/mod.hpp>

#include <iosfwd>
#include <vector>
#include <functional>

namespace solvent::lib::print {
	//
	using grid_t = std::function<uint8_t(uint32_t)>;

	[[gnu::hot]] void value(std::ostream&, Order order, uint8_t value);
	void serial(std::ostream&, Order, grid_t grid_view);
	void pretty(std::ostream&, Order, std::vector<grid_t> const& grid_views);
}
#endif