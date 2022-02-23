#ifndef HPP_SOLVENT_LIB__ORDER
#define HPP_SOLVENT_LIB__ORDER

#include <cstdint>

namespace solvent {
	using Order = unsigned;
	constexpr Order O_MAX = 10u;
	constexpr Order O2_MAX = O_MAX * O_MAX;
	constexpr Order O4_MAX = O2_MAX * O2_MAX;
}
#endif