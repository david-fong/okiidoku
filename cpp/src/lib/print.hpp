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

	[[gnu::hot]] void value(std::ostream&, Order, uint8_t);
	void serial(std::ostream&, Order, grid_t);
	void pretty(std::ostream&, Order, std::vector<grid_t> const&);

	template<Order O> void serial(std::ostream&, typename gen::Generator<O>::GenResult const&);
	template<Order O> void pretty(std::ostream&, typename gen::Generator<O>::GenResult const&);

	extern template void serial<2>(std::ostream&, typename gen::Generator<2>::GenResult const&);
	extern template void serial<3>(std::ostream&, typename gen::Generator<3>::GenResult const&);
	extern template void serial<4>(std::ostream&, typename gen::Generator<4>::GenResult const&);
	extern template void serial<5>(std::ostream&, typename gen::Generator<5>::GenResult const&);
	extern template void serial<6>(std::ostream&, typename gen::Generator<6>::GenResult const&);

	extern template void pretty<2>(std::ostream&, typename gen::Generator<2>::GenResult const&);
	extern template void pretty<3>(std::ostream&, typename gen::Generator<3>::GenResult const&);
	extern template void pretty<4>(std::ostream&, typename gen::Generator<4>::GenResult const&);
	extern template void pretty<5>(std::ostream&, typename gen::Generator<5>::GenResult const&);
	extern template void pretty<6>(std::ostream&, typename gen::Generator<6>::GenResult const&);
}
#endif