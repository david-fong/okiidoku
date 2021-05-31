#ifndef HPP_SOLVENT_LIB_PRINT
#define HPP_SOLVENT_LIB_PRINT

#include ":/lib/size.hpp"
#include ":/lib/gen/mod.hpp"

#include <iosfwd>
#include <vector>

namespace solvent::lib { class GridPrinter; }
std::ostream& operator<<(std::ostream&, solvent::lib::GridPrinter&);

namespace solvent::lib::print {
	//
	enum class Mode : unsigned {
		Serial,
		Pretty,
	};

	//
	using grid_t = uint8_t (&)(uint32_t);

	[[gnu::hot]] void value(std::ostream&, Order, uint8_t);
	void serial(std::ostream&, Order, grid_t);
	void pretty(std::ostream&, Order, std::vector<grid_t>);

	template<Order O> void serial(std::ostream&, typename gen::Generator<O>::GenResult&);
	template<Order O> void pretty(std::ostream&, typename gen::Generator<O>::GenResult&);

}
#endif