#ifndef HPP_OKIIDOKU__INTS
#define HPP_OKIIDOKU__INTS
// Note: the byte width type logic could be done with boost, but I don't
// have any other reason to add boost as a dependency, so I won't.

#include <okiidoku/detail/order_templates.hpp> // Order, largest_compiled_order

#include <bit>         // bit_width
#include <cstdint>     // uint_...
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral

namespace okiidoku {

	enum class HouseType : unsigned char {
		box, row, col,
	};
}


namespace okiidoku::mono {

	namespace detail {

		template<unsigned int N>
		using uint_fastN_t =
			std::conditional_t<(N <=   8U), std::uint_fast8_t,
			std::conditional_t<(N <=  16U), std::uint_fast16_t,
			std::conditional_t<(N <=  32U), std::uint_fast32_t,
			std::conditional_t<(N <=  64U), std::uint_fast64_t,
			// std::conditional_t<(N <= 128U), __uint128_t, // currently unused. Note: won't work with MSVC
			void
		>>>>;

		template<unsigned int N>
		using uint_smolN_t =
			std::conditional_t<(N <=   8U), std::uint_least8_t,
			std::conditional_t<(N <=  16U), std::uint_least16_t,
			std::conditional_t<(N <=  32U), std::uint_least32_t,
			std::conditional_t<(N <=  64U), std::uint_least64_t,
			// std::conditional_t<(N <= 128U), __uint128_t,
			void
		>>>>;

		// Note: this is optional based on whether the target architecture supports these fixed-width types.
		template<unsigned int N>
		using uint_fixedN_t =
			std::conditional_t<(N <=   8U), std::uint8_t,
			std::conditional_t<(N <=  16U), std::uint16_t,
			std::conditional_t<(N <=  32U), std::uint32_t,
			std::conditional_t<(N <=  64U), std::uint64_t,
			// std::conditional_t<(N <= 128U), __uint128_t,
			void
		>>>>;
	}

	// Note: when printing things, make sure to cast to int, since byte-like types will be interpreted as characters.
	template<Order O>
	struct Ints final {

		Ints() = delete;

		using o1x_t = detail::uint_fastN_t<std::bit_width(O)>;
		using o1i_t = detail::uint_fastN_t<std::bit_width(O)>;

		using o2x_t = detail::uint_fastN_t<std::bit_width(O*O-1)>;
		using o2i_t = detail::uint_fastN_t<std::bit_width(O*O)>;
		using o2x_smol_t = detail::uint_smolN_t<std::bit_width(O*O-1)>;
		using o2i_smol_t = detail::uint_smolN_t<std::bit_width(O*O)>;

		using o3i_t = detail::uint_fastN_t<std::bit_width(O*O*O)>;

		using o4x_t = detail::uint_fastN_t<std::bit_width(O*O*O*O-1)>;
		using o4i_t = detail::uint_fastN_t<std::bit_width(O*O*O*O)>;
		using o4x_smol_t = detail::uint_smolN_t<std::bit_width(O*O*O*O-1)>;
		using o4i_smol_t = detail::uint_smolN_t<std::bit_width(O*O*O*O)>;

		using o5i_t = detail::uint_fastN_t<std::bit_width(O*O*O*O*O)>;

		using o6i_t = detail::uint_fastN_t<std::bit_width(O*O*O*O*O*O)>;

		static constexpr o1i_t O1 {O};
		static constexpr o2i_t O2 {O*O};
		static constexpr o3i_t O3 {O*O*O};
		static constexpr o4i_t O4 {O*O*O*O};
	};

	// TODO.low consider changing all these allow signed integers as well? Not sure what pros and cons are.
	template<Order O, typename T>
	concept Any_o1x = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O1-1);

	template<Order O, typename T>
	concept Any_o2x = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O2-1);

	template<Order O, typename T>
	concept Any_o4x = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O4-1);


	template<Order O>
	using grid_val_t = typename Ints<O>::o2i_smol_t;
}


namespace okiidoku::visitor {

	namespace Ints {
		using o1x_t = mono::Ints<largest_compiled_order>::o1x_t;
		using o1i_t = mono::Ints<largest_compiled_order>::o1i_t;

		using o2x_t = mono::Ints<largest_compiled_order>::o2x_t;
		using o2i_t = mono::Ints<largest_compiled_order>::o2i_t;
		using o2x_smol_t = mono::Ints<largest_compiled_order>::o2x_smol_t;
		using o2i_smol_t = mono::Ints<largest_compiled_order>::o2i_smol_t;

		using o4x_t = mono::Ints<largest_compiled_order>::o4x_t;
		using o4i_t = mono::Ints<largest_compiled_order>::o4i_t;
		using o4x_smol_t = mono::Ints<largest_compiled_order>::o4x_smol_t;
		using o4i_smol_t = mono::Ints<largest_compiled_order>::o4i_smol_t;

		using o5i_t = mono::Ints<largest_compiled_order>::o5i_t;

		using o6i_t = mono::Ints<largest_compiled_order>::o6i_t;
	}
	using grid_val_t = mono::grid_val_t<largest_compiled_order>;
}
#endif