#ifndef HPP_OKIIDOKU__TRAITS
#define HPP_OKIIDOKU__TRAITS
// Note: the byte width type logic could be done with boost, but I don't
// have any other reason to add boost as a dependency, so I won't.

#include <okiidoku/detail/order_templates.hpp> // largest_compiled_order

#include <bit>
#include <bitset> // TODO.mid could this be excluded if (O_MAX <= 8) ?
#include <cstdint>
#include <limits> // numeric_limits<T>::max
#include <type_traits>
#include <concepts>

namespace okiidoku {

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
	using uint_smolN_t = 
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

}


namespace okiidoku::mono {

	// Note: when printing things, make sure to cast to int, since byte-like types will be interpreted as characters.
	template<unsigned O>
	struct traits final {

		using o1x_t = uint_fastN_t<std::bit_width(O)>;
		using o1i_t = uint_fastN_t<std::bit_width(O)>;

		using o2x_t = uint_fastN_t<std::bit_width(O*O-1)>;
		using o2i_t = uint_fastN_t<std::bit_width(O*O)>;
		using o2x_smol_t = uint_smolN_t<std::bit_width(O*O-1)>;
		using o2i_smol_t = uint_smolN_t<std::bit_width(O*O)>;

		using o4x_t = uint_fastN_t<std::bit_width(O*O*O*O-1)>;
		using o4i_t = uint_fastN_t<std::bit_width(O*O*O*O)>;
		using o4x_smol_t = uint_smolN_t<std::bit_width(O*O*O*O-1)>;
		using o4i_smol_t = uint_smolN_t<std::bit_width(O*O*O*O)>;

		using o5i_t = uint_fastN_t<std::bit_width(O*O*O*O*O)>;

		using o6i_t = uint_fastN_t<std::bit_width(O*O*O*O*O*O)>;

	private:
		// TODO consider moving this to its own header?
		// Note: this class exists instead of just using bitset because it was
		// found to have noticeably better performance for small orders.
		template<bool F/* AKA: use_fast */>
		class o2_bits {
			static constexpr bool use_int_ = O <= 8;
			using This = o2_bits<F>;
			using val_t = std::conditional_t<use_int_,
				std::conditional_t<F,
					uint_fastN_t<O*O>,
					uint_smolN_t<O*O>
				>,
				std::bitset<O*O>
			>;
			val_t val_ {};
		public:
			constexpr o2_bits(): val_{0} {}
			template<class T>
			requires std::is_integral_v<T> && (!std::is_same_v<T, val_t>) && std::is_convertible_v<T, val_t>
			constexpr o2_bits(T val): val_{static_cast<val_t>(val)} {}
			constexpr o2_bits(val_t val): val_{val} {}

			[[nodiscard, gnu::pure]] typename traits<O>::o2x_t count() const noexcept {
				if constexpr (use_int_) { return static_cast<o2x_t>(std::popcount(val_)); }
				else { return static_cast<o2x_t>(val_.count()); }
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
			[[nodiscard, gnu::pure]] This operator~() const noexcept {
				return ~val_; // TODO.high this doesn't reset the top excess bits
			}

			This& set(o2x_t at) noexcept {
				if constexpr (use_int_) { val_ |= static_cast<val_t>(val_t{1} << at); return *this; }
				else { val_.set(at); return *this; }
			}
			This& flip() noexcept {
				if constexpr (use_int_) { val_ = static_cast<val_t>(~val_) & static_cast<val_t>((val_t{1}<<O*O)-1); return *this; }
				else { val_.flip(); return *this; }
			}
			This& operator&=(const This& rhs) noexcept {
				val_ &= rhs.val_; return *this;
			}
			This& operator|=(const This& rhs) noexcept {
				val_ |= rhs.val_; return *this;
			}
			This& operator<<=(const size_t rhs) noexcept {
				val_ <<= rhs; return *this;
			}
			This& operator>>=(const size_t rhs) noexcept {
				val_ >>= rhs; return *this;
			}
			[[nodiscard, gnu::pure]] friend This operator&(This lhs, const This& rhs) noexcept {
				lhs &= rhs; return lhs;
			}
			[[nodiscard, gnu::pure]] friend This operator|(This lhs, const This& rhs) noexcept {
				lhs |= rhs; return lhs;
			}
			[[nodiscard, gnu::pure]] friend This operator<<(This lhs, const size_t rhs) noexcept {
				lhs <<= rhs; return lhs;
			}
			[[nodiscard, gnu::pure]] friend This operator>>(This lhs, const size_t rhs) noexcept {
				lhs >>= rhs; return lhs;
			}
		};
	public:
		using o2_bits_fast = o2_bits<true>;
		using o2_bits_smol = o2_bits<false>;

		static constexpr o1i_t O1 {O};
		static constexpr o2i_t O2 {O*O};
		static constexpr o4i_t O3 {O*O*O};
		static constexpr o4i_t O4 {O*O*O*O};
	};

	// TODO.low consider changing all these allow signed integers as well? Not sure what pros and cons are.
	template<Order O, typename T>
	concept Any_o4ix = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (traits<O>::O4-1);

	template<Order O, typename T>
	concept Any_o2ix = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (traits<O>::O2-1);


	template<unsigned O>
	using default_grid_val_t = traits<O>::o2i_smol_t;
}


namespace okiidoku::visitor {

	namespace traits {
		using o1x_t = mono::traits<largest_compiled_order>::o1x_t;
		using o1i_t = mono::traits<largest_compiled_order>::o1i_t;

		using o2x_t = mono::traits<largest_compiled_order>::o2x_t;
		using o2i_t = mono::traits<largest_compiled_order>::o2i_t;
		using o2x_smol_t = mono::traits<largest_compiled_order>::o2x_smol_t;
		using o2i_smol_t = mono::traits<largest_compiled_order>::o2i_smol_t;

		using o4x_t = mono::traits<largest_compiled_order>::o4x_t;
		using o4i_t = mono::traits<largest_compiled_order>::o4i_t;
		using o4x_smol_t = mono::traits<largest_compiled_order>::o4x_smol_t;
		using o4i_smol_t = mono::traits<largest_compiled_order>::o4i_smol_t;

		using o5i_t = mono::traits<largest_compiled_order>::o5i_t;

		using o6i_t = mono::traits<largest_compiled_order>::o6i_t;

		using o2_bits_fast = mono::traits<largest_compiled_order>::o2_bits_fast;
		using o2_bits_smol = mono::traits<largest_compiled_order>::o2_bits_smol;
	}
	using default_grid_val_t = mono::default_grid_val_t<largest_compiled_order>;
}
#endif