#ifndef HPP_SOLVENT_LIB__SIZE
#define HPP_SOLVENT_LIB__SIZE

#include <solvent_config.hpp>
// ^no reason to include this here except so everything else doesn't have to later.

// #include <bit> // TODO use bit_width to clean up the below
#include <cstdint>
#include <type_traits>

namespace solvent {
	//
	using Order = unsigned;
	constexpr Order MAX_REASONABLE_ORDER = 10u;

	// Note: when printing, make sure to cast uint8_t to int.
	template<Order O>
	struct size final {
	 public:
		// mask width `order^2` bits.
		// O1:     2   3   4   5   6   7   8   9  10  11
		// O2:     4   9  16  25  36  49  64  81 100 121
		// round:  8  16  16  32  64  64  64 128 128 128
		using O2_mask_fast_t =
			std::conditional_t<(O <= 2), std::uint_fast8_t,
			std::conditional_t<(O <= 4), std::uint_fast16_t,
			std::conditional_t<(O <= 5), std::uint_fast32_t,
			std::conditional_t<(O <= 8), std::uint_fast64_t,
			unsigned long long
		>>>>; // std::bitset not used for performance reasons

		using O2_mask_least_t =
			std::conditional_t<(O <= 2), std::uint_least8_t,
			std::conditional_t<(O <= 4), std::uint_least16_t,
			std::conditional_t<(O <= 5), std::uint_least32_t,
			std::conditional_t<(O <= 8), std::uint_least64_t,
			unsigned long long
		>>>>;

		// uint range [0, order].
		// not used in any critical code, so it doesn't need to be fast type.
		using ord1_t = std::uint_fast8_t;

		// uint range [0, order^2].
		// O1:   2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
		// O1:   4    9   16   25   36   49   64   81  100  121  144  169  196  225  256
		// bits: 3    3    5    5    6    6    7    7    7    7    8    8    8    8    9
		using ord2_t =
			std::conditional_t<(O < 16), std::uint_fast8_t,
			std::uint_fast16_t
		>;

		// uint range [0, order^4].
		// O1:    2    3    4    5     6     7     8     9     10  ...     16
		// O4:   16   81  256  625  1296  2401  4096  6561  10000  ...  65536
		// bits:  5    7    9    9    11    12    13    13     14  ...     17
		using ord4_t =
			std::conditional_t<(O <   4), std::uint_fast8_t,
			std::conditional_t<(O <  16), std::uint_fast16_t,
			std::conditional_t<(O < 256), std::uint_fast32_t,
			std::uint_fast64_t
		>>>;
		using ord4_least_t =
			std::conditional_t<(O <   4), std::uint_least8_t,
			std::conditional_t<(O <  16), std::uint_least16_t,
			std::conditional_t<(O < 256), std::uint_least32_t,
			std::uint_least64_t
		>>>;

		// uint range [0, order^5].
		// O1:    2    3     4     5     6      7      8      9     10
		// O4:   32  243  1024  3125  7776  16807  32768  59049  10000
		// bits:  6    8    11    12    13     15     16     16     17
		using ord5_t =
			std::conditional_t<(O <=  3), std::uint_fast8_t,
			std::conditional_t<(O <=  9), std::uint_fast16_t,
			std::conditional_t<(O <= 84), std::uint_fast32_t,
			std::uint_fast64_t
		>>>;
	};
}
#endif