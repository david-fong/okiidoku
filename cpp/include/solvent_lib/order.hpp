#ifndef HPP_SOLVENT_LIB__ORDER
#define HPP_SOLVENT_LIB__ORDER

#include <solvent_config.hpp>
#include <cstdint>

namespace solvent {
	using Order = unsigned;
	constexpr Order O_MAX = M_SOLVENT_O_MAX;
	constexpr Order O2_MAX = O_MAX * O_MAX;
	constexpr Order O4_MAX = O2_MAX * O2_MAX;
}
#endif