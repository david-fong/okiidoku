The following fails to compile on GCC 14:

```cpp
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
namespace okiidoku {
	using Order = unsigned;
}
#include <bit>         // bit_width
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral
#include <cstdint>     // uint_...

namespace okiidoku::mono {
	namespace detail {
		template<int N>
		using uint_fastN_t =
			std::conditional_t<(N <=   8U), std::uint_fast8_t,
			std::conditional_t<(N <=  16U), std::uint_fast16_t,
			std::conditional_t<(N <=  32U), std::uint_fast32_t,
			std::conditional_t<(N <=  64U), std::uint_fast64_t,
			// std::conditional_t<(N <= 128U), __uint128_t, // currently unused. Note: won't work with MSVC
			void
		>>>>;
	}
	namespace int_ts {
		template<Order O> using o2i_t = detail::uint_fastN_t<std::bit_width(O*O)>;
		template<Order O> using o4x_t = detail::uint_fastN_t<std::bit_width(O*O*O*O-1)>;
	}
	template<Order O>
	struct Ints final {
		Ints() = delete;
		static constexpr Ints<O>::o2i_t O2 {O*O};
	};
	template<Order O, typename T>
	concept Any_o2x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O2-1);

	template<Order O, class T_row, class T_col>
	requires(Any_o2x_t<O, T_row>, Any_o2x_t<O, T_col>) [[nodiscard, gnu::const]]
	constexpr Ints<O>::o4x_t row_col_to_rmi(const T_row row, const T_col col) noexcept {
		return 0;
	}
}
```

With diagnostic message:

```cpp
In file included from /opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/bit:36,
                 from <source>:6:
/opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/concepts:107:13:   required for the satisfaction of 'integral<_Tp>' [with _Tp = T_row]
/opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/concepts:113:13:   required for the satisfaction of 'unsigned_integral<T>' [with T = T_row]
/opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/concepts:107:24: error: the value of 'std::is_integral_v<T_row>' is not usable in a constant expression
  107 |     concept integral = is_integral_v<_Tp>;
      |                        ^~~~~~~~~~~~~~~~~~
In file included from /opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/concepts:46:
/opt/compiler-explorer/gcc-14.2.0/include/c++/14.2.0/type_traits:3241:25: note: 'std::is_integral_v<T_row>' used in its own initializer
 3241 |   inline constexpr bool is_integral_v = is_integral<_Tp>::value;
      |                         ^~~~~~~~~~~~~
Compiler returned: 1
```

Clang 19 compiles it happily and it does what I expect. The problem is that `template<...> requires(Any_o2x_t<O, T_row>, Any_o2x_t<O, T_col>)` should be `template<...> requires(Any_o2x_t<O, T_row> && Any_o2x_t<O, T_col>)`. Now my question is... why did GCC accept this before version 14? And why does clang 19 still accept it? And where the heck did I even get this from? Did I just pull it out of thin air? Did I see it somewhere on the web? Arghhhhhhhhhhhh. But at least I figured it out now and can get on with things...