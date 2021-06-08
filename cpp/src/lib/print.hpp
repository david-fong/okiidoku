#ifndef HPP_SOLVENT_LIB_PRINT
#define HPP_SOLVENT_LIB_PRINT

#include ":/lib/size.hpp"
#include ":/lib/gen/mod.hpp"

#include <iosfwd>
#include <vector>
#include <functional>

namespace solvent::lib::print {
	//
	using grid_t = std::function<uint8_t(uint32_t)>;

	[[gnu::hot]] void value(std::ostream&, Order order, uint8_t value);
	void serial(std::ostream&, Order, grid_t grid_view);
	void pretty(std::ostream&, Order, std::vector<grid_t> const& grid_views);

	template<Order O> void serial(std::ostream&, typename gen::Generator<O>::GenResult const&);
	template<Order O> void pretty(std::ostream&, typename gen::Generator<O>::GenResult const&);

	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template void serial<O_>(std::ostream&, typename gen::Generator<O_>::GenResult const&); \
		extern template void pretty<O_>(std::ostream&, typename gen::Generator<O_>::GenResult const&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif