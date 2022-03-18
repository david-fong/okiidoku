#ifndef HPP_SOLVENT__SIZE
#define HPP_SOLVENT__SIZE
// Note: the byte width type logic could be done with boost, but I don't
// have any other reason to add boost as a dependency, so I won't.

#include "solvent/order.hpp"

#include <bit>
#include <bitset>
#include <cstdint>
#include <type_traits>

namespace solvent {

	template<int N>
	using uint_fastN_t = 
		std::conditional_t<(N <=   8), std::uint_fast8_t,
		std::conditional_t<(N <=  16), std::uint_fast16_t,
		std::conditional_t<(N <=  32), std::uint_fast32_t,
		std::conditional_t<(N <=  64), std::uint_fast64_t,
		// std::conditional_t<(N <= 128), __uint128_t, // currently unused. Note: won't work with MSVC
		void
	>>>>;

	template<int N>
	using uint_leastN_t = 
		std::conditional_t<(N <=   8), std::uint_least8_t,
		std::conditional_t<(N <=  16), std::uint_least16_t,
		std::conditional_t<(N <=  32), std::uint_least32_t,
		std::conditional_t<(N <=  64), std::uint_least64_t,
		// std::conditional_t<(N <= 128), __uint128_t,
		void
	>>>>;

	// Note: this is optional based on whether the target architecture supports these fixed-width types.
	template<int N>
	using uint_fixedN_t = 
		std::conditional_t<(N <=   8), std::uint8_t,
		std::conditional_t<(N <=  16), std::uint16_t,
		std::conditional_t<(N <=  32), std::uint32_t,
		std::conditional_t<(N <=  64), std::uint64_t,
		// std::conditional_t<(N <= 128), __uint128_t,
		void
	>>>>;


	// Note: when printing, make sure to cast uint8_t to int.
	template<Order O>
	struct size final {
	private:
		template<bool F/* AKA: use_fast */>
		class O2_mask_t {
			static constexpr bool use_int_ = O <= 8;
			using val_t = std::conditional_t<use_int_,
				std::conditional_t<F,
					uint_fastN_t<O*O>,
					uint_leastN_t<O*O>
				>,
				std::bitset<O*O>
			>;
			val_t val_ {};
		public:
			constexpr O2_mask_t(): val_{0} {}
			template<class T>
			requires std::is_integral_v<T> && (!std::is_same_v<T, val_t>) && std::is_convertible_v<T, val_t>
			constexpr O2_mask_t(T val): val_{static_cast<val_t>(val)} {}
			constexpr O2_mask_t(val_t val): val_{val} {}

			[[nodiscard, gnu::pure]] typename size<O>::ord2x_t count() const noexcept {
				if constexpr (use_int_) { return static_cast<ord2x_t>(std::popcount(val_)); }
				else { return static_cast<ord2x_t>(val_.count()); }
			}
			[[nodiscard, gnu::pure]] bool all() const noexcept {
				if constexpr (use_int_) { return std::popcount(val_) == O*O; } else { return val_.all(); }
			}
			[[nodiscard, gnu::pure]] bool any() const noexcept {
				if constexpr (use_int_) { return val_ != 0; } else { return val_.any(); }
			}
			[[nodiscard, gnu::pure]] bool none() const noexcept {
				if constexpr (use_int_) { return val_ == 0; } else { return val_.none(); }
			}
			[[nodiscard, gnu::pure]] O2_mask_t<F> operator~() const noexcept {
				return ~val_;
			}

			O2_mask_t<F>& operator&=(const O2_mask_t<F>& rhs) noexcept {
				val_ &= rhs.val_; return *this;
			}
			O2_mask_t<F>& operator|=(const O2_mask_t<F>& rhs) noexcept {
				val_ |= rhs.val_; return *this;
			}
			O2_mask_t<F>& operator<<=(const size_t rhs) noexcept {
				val_ <<= rhs; return *this;
			}
			[[nodiscard, gnu::pure]] friend O2_mask_t<F> operator&(O2_mask_t<F> lhs, const O2_mask_t<F>& rhs) noexcept {
				lhs &= rhs; return lhs;
			}
			[[nodiscard, gnu::pure]] friend O2_mask_t<F> operator|(O2_mask_t<F> lhs, const O2_mask_t<F>& rhs) noexcept {
				lhs |= rhs; return lhs;
			}
			[[nodiscard, gnu::pure]] friend O2_mask_t<F> operator<<(O2_mask_t<F> lhs, const size_t rhs) noexcept {
				lhs <<= rhs; return lhs;
			}
		};
	public:
		// mask width `order^2` bits.
		// O1:     2   3   4   5   6   7   8   9  10  11
		// O2:     4   9  16  25  36  49  64  81 100 121
		// round:  8  16  16  32  64  64  64 128 128 128
		// std::bitset not used because it's apparently not fast
		using O2_mask_fast_t = O2_mask_t<true>;
		using O2_mask_least_t = O2_mask_t<false>;

		// uint range [0, order].
		using ord1i_t = uint_fastN_t<std::bit_width(O)>;

		// uint range [0, order^2].
		// O1:   2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
		// O2:   4    9   16   25   36   49   64   81  100  121  144  169  196  225  256
		// bits: 3    3    5    5    6    6    7    7    7    7    8    8    8    8    9
		using ord2x_t = uint_fastN_t<std::bit_width(O*O-1)>;
		using ord2i_t = uint_fastN_t<std::bit_width(O*O)>;
		using ord2x_least_t = uint_leastN_t<std::bit_width(O*O-1)>;
		using ord2i_least_t = uint_leastN_t<std::bit_width(O*O)>;
		using ord2x_fixed_t = uint_fixedN_t<std::bit_width(O*O-1)>;
		using ord2i_fixed_t = uint_fixedN_t<std::bit_width(O*O)>;

		// uint range [0, order^4].
		// O1:    2    3    4    5     6     7     8     9     10  ...     16
		// O4:   16   81  256  625  1296  2401  4096  6561  10000  ...  65536
		// bits:  5    7    9    9    11    12    13    13     14  ...     17
		using ord4x_t = uint_fastN_t<std::bit_width(O*O*O*O-1)>;
		using ord4i_t = uint_fastN_t<std::bit_width(O*O*O*O)>;
		using ord4x_least_t = uint_leastN_t<std::bit_width(O*O*O*O-1)>;
		using ord4i_least_t = uint_leastN_t<std::bit_width(O*O*O*O)>;

		// uint range [0, order^5].
		// O1:    2    3     4     5     6      7      8      9     10
		// O4:   32  243  1024  3125  7776  16807  32768  59049  10000
		// bits:  6    8    11    12    13     15     16     16     17
		using ord5i_t = uint_fastN_t<std::bit_width(O*O*O*O*O)>;

		using ord6i_t = uint_fastN_t<std::bit_width(O*O*O*O*O*O)>;
	};
}
#endif