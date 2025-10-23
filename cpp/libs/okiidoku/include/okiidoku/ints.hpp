// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_INTS
#define HPP_OKIIDOKU_INTS

#include <okiidoku/order.hpp> // Order, largest_compiled_order
#include <okiidoku/detail/util.hpp>

#include <array>
#include <utility>     // forward_like, to_underlying
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral
#include <compare>     //
#include <cstdint>     // uint_...

// TODO.low would it be useful to add `gnu::artificial` for some of these functions? https://stackoverflow.com/a/21936099/11107541

/// \cond detail
namespace emscripten::internal { template<typename T, typename> struct BindingType; }
/// \endcond detail

/** top-level namespace for the okiidoku project. */
namespace okiidoku {

	using rng_seed_t = std::uint_fast32_t; // see `std::minstd_rand`

	enum class HouseType : unsigned char {
		box, row, col,
	};
	inline constexpr auto house_types {std::to_array<HouseType>({
		HouseType::row,
		HouseType::col,
		HouseType::box,
	})};
	template<typename V> requires(!std::is_reference_v<V>)
	struct HouseTypeMap final {
		std::array<V, house_types.size()> arr {};
		[[nodiscard, gnu::pure]]
		decltype(auto) operator[](this auto&& self, const HouseType key) noexcept {
			return std::forward_like<decltype(self)>(self.arr[std::to_underlying(key)]);
		}
	};

	enum class LineType : unsigned char {
		row, col,
	};
	inline constexpr auto line_types {std::to_array<LineType>({
		LineType::row,
		LineType::col,
	})};
	template<typename V> requires(!std::is_reference_v<V>)
	struct LineTypeMap final {
		std::array<V, line_types.size()> arr {};
		[[nodiscard, gnu::pure]]
		decltype(auto) operator[](this auto&& self, const LineType key) noexcept {
			return std::forward_like<decltype(self)>(self.arr[std::to_underlying(key)]);
		}
	};

	enum class BoxOrLine : unsigned char {
		box, line,
	};

	enum class CellOrSym : unsigned char {
		cell, sym,
	};
}


namespace okiidoku {

	namespace detail {

		template<int W> requires(W <= 64)
		/// fast unsigned integer type that can fit at least `N` bits.
		using uint_fast_for_width_t = typename
			std::conditional_t<(W <= 8 ), std::uint_fast8_t ,
			std::conditional_t<(W <= 16), std::uint_fast16_t,
			std::conditional_t<(W <= 32), std::uint_fast32_t,
			std::conditional_t<(W <= 64), std::uint_fast64_t,
			// std::conditional_t<(W <= 128u), __uint128_t, // currently unused. Note: won't work with MSVC
			void
		>>>>;

		template<int W> requires(W <= 64)
		/// smallest unsigned integer type that can fit at least `N` bits.
		using uint_small_for_width_t = typename
			std::conditional_t<(W <= 8 ), std::uint_least8_t ,
			std::conditional_t<(W <= 16), std::uint_least16_t,
			std::conditional_t<(W <= 32), std::uint_least32_t,
			std::conditional_t<(W <= 64), std::uint_least64_t,
			// std::conditional_t<(W <= 128u), __uint128_t,
			void
		>>>>;

		template<std::uintmax_t M>
		/// fast unsigned integer type that can fit max value `M`.
		using uint_fast_for_max_t = typename
			std::conditional_t<(M <= UINT_LEAST8_MAX ), std::uint_fast8_t ,
			std::conditional_t<(M <= UINT_LEAST16_MAX), std::uint_fast16_t,
			std::conditional_t<(M <= UINT_LEAST32_MAX), std::uint_fast32_t,
			std::conditional_t<(M <= UINT_LEAST64_MAX), std::uint_fast64_t,
			void
		>>>>;

		template<std::uintmax_t M>
		/// smallest unsigned integer type that can fit max value `M`.
		using uint_small_for_max_t = typename
			std::conditional_t<(M <= UINT_LEAST8_MAX ), std::uint_least8_t ,
			std::conditional_t<(M <= UINT_LEAST16_MAX), std::uint_least16_t,
			std::conditional_t<(M <= UINT_LEAST32_MAX), std::uint_least32_t,
			std::conditional_t<(M <= UINT_LEAST64_MAX), std::uint_least64_t,
			void
		>>>>;

		template<std::intmax_t M>
		/// fast signed integer type that can fit max value `M`.
		using int_fast_for_max_t = typename
			std::conditional_t<(M <= INT_LEAST8_MAX ), std::int_fast8_t ,
			std::conditional_t<(M <= INT_LEAST16_MAX), std::int_fast16_t,
			std::conditional_t<(M <= INT_LEAST32_MAX), std::int_fast32_t,
			std::conditional_t<(M <= INT_LEAST64_MAX), std::int_fast64_t,
			void
		>>>>;

		template<std::intmax_t M>
		/// smallest signed integer type that can fit max value `M`.
		using int_small_for_max_t = typename
			std::conditional_t<(M <= INT_LEAST8_MAX ), std::int_least8_t ,
			std::conditional_t<(M <= INT_LEAST16_MAX), std::int_least16_t,
			std::conditional_t<(M <= INT_LEAST32_MAX), std::int_least32_t,
			std::conditional_t<(M <= INT_LEAST64_MAX), std::int_least64_t,
			void
		>>>>;

		// template<int N>
		// /// unsigned integer type that can fits an exact-byte-multiple of `N` bits.
		// /// \note this is optional based on whether the target architecture supports these fixed-width types.
		// using uint_fixedN_t = typename
		// 	std::conditional_t<(N <=   8), std::uint8_t,
		// 	std::conditional_t<(N <=  16), std::uint16_t,
		// 	std::conditional_t<(N <=  32), std::uint32_t,
		// 	std::conditional_t<(N <=  64), std::uint64_t,
		// 	// std::conditional_t<(N <= 128u), __uint128_t,
		// 	void
		// >>>>;
	}


	/** this tag determines the space usage and valid operations for an `Int`. */
	enum class IntKind : unsigned char {
		fixed,
		// TODO consider another type "argument", (only-)which has the implicit constructor from builtin type.
		small,
		fast,
	};
	/// \cond detail
	consteval IntKind pick_int_op_result_kind(const IntKind k1, const IntKind k2) noexcept {
		using enum IntKind;
		if (k1 == fixed) { return k2; }
		if (k2 == fixed) { return k1; }
		if (k1 == small && k2 == small) { return small; }
		return fast;
	}
	/// \endcond detail

	/** unsigned integer wrapper class with inclusive upper bound.
	iterable with the upper bound as an excluded sentinel.
	\note for `std::ostream` support, `#include <okiidoku/ints_io.hpp>`. */
	template<std::uintmax_t max_, IntKind kind_ = IntKind::fast>
	class Int final {
		template<std::uintmax_t M2, IntKind K2> friend class Int;
		friend struct emscripten::internal::BindingType<Int, std::true_type>;
		using max_t = std::uintmax_t;
	public:
		static constexpr max_t   max  {max_}; ///< inclusive upper bound.
		static constexpr IntKind kind {kind_};
		/** underlying value type. */
		using val_t =
			std::conditional_t<kind == IntKind::fixed, detail::uint_fast_for_max_t <max>,
			std::conditional_t<kind == IntKind::small, detail::uint_small_for_max_t<max>,
			std::conditional_t<kind == IntKind::fast,  detail::uint_fast_for_max_t <max>,
			void
		>>>;
			static_assert(std::is_unsigned_v<val_t>);
			static_assert(std::numeric_limits<val_t>::max() >= max, "has enough capacity");
		using difference_type = detail::int_fast_for_max_t<static_cast<std::intmax_t>(max)>;
	private:
		/** underlying storage. */
		[[no_unique_address]] std::conditional_t<(kind==IntKind::fixed), bool, val_t> val_ {0u};

		explicit consteval Int([[maybe_unused]] const std::uintmax_t val) noexcept requires(kind == IntKind::fixed) { OKIIDOKU_CONTRACT(val == max); }
	public:
		[[gnu::always_inline]] constexpr void check() const noexcept requires(kind == IntKind::fixed) {}
		[[gnu::always_inline]] constexpr void check() const noexcept requires(kind != IntKind::fixed) {
			OKIIDOKU_CONTRACT(val_ <= static_cast<val_t>(max));
			OKIIDOKU_CONTRACT(val_ <= max);
		}
		constexpr Int() noexcept = default;

		/** construct from builtin int type.
		\pre `0 <= in <= max` */
		template<std::integral T_in>
		constexpr Int(const T_in in) noexcept requires(kind != IntKind::fixed): val_{static_cast<val_t>(in)} { // NOLINT(*-explicit-constructor)
			if constexpr (std::is_signed_v<T_in>) {
				OKIIDOKU_CONTRACT(in >= 0);
			}
			OKIIDOKU_CONTRACT(static_cast<max_t>(in) <= max);
			check();
		}

		/** implicit conversion from narrower / same-capacity `Int`. */
		template<max_t M_from, IntKind K_from>
		constexpr Int(const Int<M_from,K_from> other) noexcept requires(kind != IntKind::fixed && M_from <= max): // NOLINT(*-explicit-constructor)
			val_{static_cast<val_t>(other.val())} { check(); }

		/** construct from another bounded int with higher capacity.
		\pre `other.val() <= max`. */
		template<max_t M_from, IntKind K_from> [[gnu::const]]
		static constexpr Int unchecked_from(const Int<M_from,K_from> other) noexcept requires(kind != IntKind::fixed) {
			OKIIDOKU_CONTRACT(other.val() <= max);
			return Int{other.val()};
		}

		/** implicit conversion to a fast, builtin, unsigned integer type with enough capacity. */
		[[nodiscard, gnu::const]] constexpr operator detail::uint_fast_for_max_t<max>(this const Int self) noexcept { // NOLINT(*-explicit-constructor)
			self.check(); return detail::uint_fast_for_max_t<max>{self.val()};
		}
		// template<class T> requires(std::unsigned_integral<T> && std::numeric_limits<T>::max() >= max)
		// constexpr operator T() const noexcept { check(); return val(); }
		[[nodiscard, gnu::const]] constexpr val_t val([[maybe_unused]] this const Int self) noexcept requires(kind == IntKind::fixed) { return max; }
		[[nodiscard, gnu::const]] constexpr val_t val(                 this const Int self) noexcept requires(kind != IntKind::fixed) { self.check(); return self.val_; }
		[[nodiscard, gnu::const]] constexpr auto   as_fast(this const Int self) noexcept { self.check(); return Int<max, IntKind::fast> {self.val()}; }
		[[nodiscard, gnu::const]] constexpr auto  as_small(this const Int self) noexcept { self.check(); return Int<max, IntKind::small>{self.val()}; }
		using constant_t = Int<max_, IntKind::fixed>;

		// see ints_io.hpp
		// friend std::ostream& operator<<(std::ostream& os, const Int& i) noexcept;

		/** \pre `val() < max`. */ constexpr auto& operator++(   ) & noexcept requires(kind != IntKind::fixed) { OKIIDOKU_CONTRACT(val() < max);       ++val_; check(); return *this; }
		/** \pre `val() > 0u`.  */ constexpr auto& operator--(   ) & noexcept requires(kind != IntKind::fixed) { OKIIDOKU_CONTRACT(val() > val_t{0u}); --val_; check(); return *this; }
		/** \pre `val() < max`. */ constexpr auto  operator++(int) & noexcept requires(kind != IntKind::fixed) { auto old {*this}; operator++(); return old; }
		/** \pre `val() > 0u`.  */ constexpr auto  operator--(int) & noexcept requires(kind != IntKind::fixed) { auto old {*this}; operator--(); return old; }

		[[nodiscard, gnu::const]] constexpr auto begin(this const Int self) noexcept { self.check(); return Int<max,IntKind::fast>{0u}; }
		[[nodiscard, gnu::const]] constexpr auto end  (this const Int self) noexcept { self.check(); return self.as_fast(); }
		/** \pre `val() < max`. \note for increment that also increments capacity, `+ int1`. */
		[[nodiscard, gnu::const]] constexpr auto next (this const Int self) noexcept requires(kind != IntKind::fixed) { self.check(); OKIIDOKU_CONTRACT(self.val_ < max); return Int{self.val()+1u}; }
		/** \pre `val() > 0u`. */
		[[nodiscard, gnu::const]] constexpr auto prev (this const Int self) noexcept requires(max > 0u) { if constexpr (kind != IntKind::fixed) { self.check(); OKIIDOKU_CONTRACT(self.val() >= val_t{0u}); return Int<max-1u,kind>{self.val()-1u}; } else { return Int<max-1u,kind>(); } }
		/** \pre `val() < max`. */
		[[nodiscard, gnu::const]] constexpr Int<max-1u,kind> operator*(this const Int self) noexcept requires(kind != IntKind::fixed) {
			static_assert(max > 0u);
			self.check(); OKIIDOKU_CONTRACT(self.val() < max);
			return Int<max-1u,kind>{self.val()};
		}

		/** \pre `val() + other.val() <= max` */
		template<max_t MR, IntKind KR>
		constexpr Int& operator+=(const Int<MR,KR> other) & noexcept requires(kind != IntKind::fixed && MR <= max) {
			check(); other.check();
			OKIIDOKU_CONTRACT(max_t{other.val()} <= max);
			OKIIDOKU_CONTRACT(max_t{val()} <= max_t{max - max_t{other.val()}}); // (no) overflow check. i.e. `val() + other.val() <= max`
			OKIIDOKU_CONTRACT(max_t{val()} + other.val() <= max); // (no) overflow check
			val_ += other.val();
			check();
			return *this;
		}

		/** \pre `val() >= >= other.val()` */
		template<max_t MR, IntKind KR>
		constexpr Int& operator-=(const Int<MR,KR> other) & noexcept requires(kind != IntKind::fixed && MR <= max) {
			check(); other.check(); OKIIDOKU_CONTRACT(other.val() <= val());
			val_ -= other.val();
			check();
			return *this;
		}

		/** \pre `other.val() != 0` */
		template<max_t MR, IntKind KR>
		constexpr Int& operator/=(const Int<MR,KR> other) & noexcept requires(kind != IntKind::fixed && MR > 0u && MR <= max) {
			check(); other.check(); OKIIDOKU_CONTRACT(other.val() != val_t{0u});
			val_ /= other.val();
			check();
			return *this;
		}

		// \internal can't define in here- it would cause redefinitions.
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend std::strong_ordering operator<=>(Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator==(Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator!=(Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator<=(Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator< (Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator>=(Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator> (Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator+ (Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator- (Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept requires(ML >= MR);
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator* (Int<ML,KL> lhs, Int<MR,KR> rhs) noexcept requires(ML==0u || MR==0u || ML <= std::numeric_limits<std::uintmax_t>::max() / MR);
		template<max_t ML, IntKind KL, max_t MR            > constexpr friend auto operator/ (Int<ML,KL> lhs, Int<MR,IntKind::fixed>   rhs) noexcept requires(MR > 0u);
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator% (Int<ML,KL> lhs, Int<MR,KR>                  rhs) noexcept requires(MR > 0u);
		template<                      max_t MR, IntKind KR> constexpr friend auto operator% (std::unsigned_integral auto lhs, Int<MR,KR> rhs) noexcept requires(MR > 0u);
	};

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr std::strong_ordering operator<=>(const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() <=> rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator== (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() == rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator!= (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() != rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator<= (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() <= rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator<  (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() <  rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator>= (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() >= rhs.val(); }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::const]] constexpr bool operator>  (const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept { lhs.check(); rhs.check(); return lhs.val() >  rhs.val(); }

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::const]] constexpr auto operator+(const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept requires(ML <= std::numeric_limits<std::uintmax_t>::max() - MR) {
		lhs.check(); rhs.check();
		static constexpr auto KO {pick_int_op_result_kind(KL,KR)};
		return Int<ML+MR,KO>{lhs.val() + rhs.val()};
	}
	// TODO what happens if it would overflow? does the compiler implicitly convert the ints to the builtin types? how could I test against that, and prevent it?

	/**
	\pre `lhs >= rhs`
	\note uses size of `lhs`. doesn't assume minimum value of `rhs`. */
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::const]] constexpr auto operator-(const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept requires(ML >= MR) {
		lhs.check(); rhs.check();
		OKIIDOKU_CONTRACT(lhs.val() >= rhs.val());
		static constexpr auto KO {pick_int_op_result_kind(KL,KR)};
		return Int<ML,KO>{lhs.val() - rhs.val()};
	}

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::const]] constexpr auto operator*(const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept requires(ML==0u || MR==0u || ML <= std::numeric_limits<std::uintmax_t>::max() / MR) {
		lhs.check(); rhs.check();
		static constexpr auto KO {pick_int_op_result_kind(KL,KR)};
		Int<ML*MR,KO> ret {static_cast<Int<ML*MR,KO>::val_t>(lhs.val() * rhs.val())};
		OKIIDOKU_CONTRACT((lhs.val() == 0u || rhs.val() == 0u) == (ret.val() == 0u));
		OKIIDOKU_CONTRACT((lhs.val() == 1u || rhs.val() == 0u) == (ret.val() == rhs.val()));
		OKIIDOKU_CONTRACT((rhs.val() == 1u || lhs.val() == 0u) == (ret.val() == lhs.val()));
		OKIIDOKU_CONTRACT((lhs.val() >  1u && rhs.val() >  0u) == (ret.val() >  rhs.val()));
		OKIIDOKU_CONTRACT((rhs.val() >  1u && lhs.val() >  0u) == (ret.val() >  lhs.val()));
		return ret;
	}
	// TODO what happens if it would overflow? does the compiler implicitly convert the ints to the builtin types? how could I test against that, and prevent it?

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR>
	[[gnu::const]] constexpr auto operator/(const Int<ML,KL> lhs, const Int<MR,IntKind::fixed> rhs) noexcept requires(MR > 0u) {
		lhs.check(); rhs.check();
		OKIIDOKU_CONTRACT(rhs.val() == MR);
		OKIIDOKU_CONTRACT(rhs.val() > 0u);
		return Int<ML/MR,IntKind::fast>{static_cast<detail::uint_fast_for_max_t<ML>>(lhs.val() / MR)};
	}

	/** \pre `rhs > 0` */
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::const]] constexpr auto operator%(const Int<ML,KL> lhs, const Int<MR,KR> rhs) noexcept requires(MR > 0u) {
		lhs.check(); rhs.check(); OKIIDOKU_CONTRACT(rhs.val() > 0u);
		static constexpr auto KO {pick_int_op_result_kind(KL,KR)};
		Int<std::min(ML,MR-1u),KO> ret {lhs.val() % rhs.val()};
		OKIIDOKU_CONTRACT(ret.val() < rhs.val());
		OKIIDOKU_CONTRACT(ret < rhs);
		return ret;
	}
	/** \pre `rhs > 0` */
	template<std::uintmax_t MR, IntKind KR>
	[[gnu::const]] constexpr auto operator%(const std::unsigned_integral auto lhs, const Int<MR,KR> rhs) noexcept requires(MR > 0u) {
		rhs.check();
		OKIIDOKU_CONTRACT(rhs.val() > 0u);
		Int<MR-1u,IntKind::fast> ret {lhs % rhs.val()};
		OKIIDOKU_CONTRACT(ret.val() < rhs.val());
		OKIIDOKU_CONTRACT(ret < rhs);
		return ret;
	}

	inline constexpr Int<1u, IntKind::fixed> int1 {};

	namespace detail {
		/** compile-time helper for `bounded_int`. */
		template<std::uintmax_t max_, IntKind kind_> [[gnu::unavailable]]
		consteval void accept_int_wrapper([[maybe_unused]] Int<max_, kind_>) noexcept {}

		// used by emscripten bindings for "value type" definition
		template<typename T>
		concept bounded_int = requires { accept_int_wrapper(std::declval<T>()); };
	}
}
namespace std {
	/**
	\internal
	non-standard libraries may add specializations for library-provided types
	https://en.cppreference.com/w/cpp/types/numeric_limits.html

	specializations of `std::numeric_limits` must define all members declared `static constexpr`
	in the primary template, in such a way that they are usable as integral constant expressions.
	https://en.cppreference.com/w/cpp/language/extending_std.html
	*/
	template<okiidoku::detail::bounded_int I>
	struct numeric_limits<I> : public numeric_limits<typename I::val_t> { // NOLINT(cert-dcl58-cpp)
		static constexpr bool is_modulo {false}; // I haven't implemented that wrapping
		static constexpr I max() noexcept { return I{I::max}; }
	};
}
// namespace Catch {
// 	template<typename T> requires(::okiidoku::detail::bounded_int<T>)
// 	static constexpr bool is_range<T>::value {false};
// }


/**
this namespace is for the per-grid-order definitions of okiidoku library features.
most things are templatized based on grid order to optimize memory usage (at the
expense of generating code for each grid order selected by the user). to choose what
grid orders to generate, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header)
for `./libs/okiidoku/include/okiidoku/config/defaults.hpp`.
*/
namespace okiidoku::mono {
	/** a class acting as a namespace of sized int types and constants.
	\details do `using T = Ints<O>;`. */
	template<Order O> requires(is_order_compiled(O))
	struct Ints final {
		Ints() = delete;

		#define DEFINE_OX_TYPES(P_, SUFFIX_, MAX_) \
		using o##P_##SUFFIX_##_t  = Int<std::uintmax_t{(MAX_)}, IntKind::fast >; \
		using o##P_##SUFFIX_##s_t = Int<std::uintmax_t{(MAX_)}, IntKind::small>; \
		\
		static_assert(sizeof(o##P_##SUFFIX_## _t) == sizeof(typename o##P_##SUFFIX_## _t::val_t)); \
		static_assert(sizeof(o##P_##SUFFIX_##s_t) == sizeof(typename o##P_##SUFFIX_##s_t::val_t)); \
		static_assert(detail::bounded_int<o##P_##SUFFIX_## _t>); \
		static_assert(detail::bounded_int<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_trivially_copyable_v<o##P_##SUFFIX_## _t>); \
		static_assert( std::is_trivially_copyable_v<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_standard_layout_v<o##P_##SUFFIX_## _t>); \
		static_assert( std::is_standard_layout_v<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_trivially_copy_assignable_v<o##P_##SUFFIX_## _t>); \
		static_assert( std::is_trivially_copy_assignable_v<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_trivially_move_assignable_v<o##P_##SUFFIX_## _t>); \
		static_assert( std::is_trivially_move_assignable_v<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_nothrow_convertible_v<o##P_##SUFFIX_## _t, o##P_##SUFFIX_##s_t>, "can convert fast to small"); \
		static_assert( std::is_nothrow_convertible_v<o##P_##SUFFIX_##s_t, o##P_##SUFFIX_## _t>, "can convert small to fast"); \
		static_assert( std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)-1u}>,o##P_##SUFFIX_## _t>, "can convert from narrower Int"); \
		static_assert( std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)-1u}>,o##P_##SUFFIX_##s_t>, "can convert from narrower Int"); \
		static_assert(!std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)+1u}>,o##P_##SUFFIX_## _t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)+1u}>,o##P_##SUFFIX_##s_t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)+1u},IntKind::small>,o##P_##SUFFIX_## _t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<std::uintmax_t{(MAX_)+1u},IntKind::small>,o##P_##SUFFIX_##s_t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_constructible_v<o##P_##SUFFIX_## _t,Int<std::uintmax_t{(MAX_)+1u}>>, "explicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_constructible_v<o##P_##SUFFIX_##s_t,Int<std::uintmax_t{(MAX_)+1u}>>, "explicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_constructible_v<o##P_##SUFFIX_## _t,Int<std::uintmax_t{(MAX_)+1u},IntKind::small>>, "explicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_constructible_v<o##P_##SUFFIX_##s_t,Int<std::uintmax_t{(MAX_)+1u},IntKind::small>>, "explicit conversion from sentinel type"); \
		static_assert( std::is_nothrow_assignable_v<o##P_##SUFFIX_## _t,unsigned char>); \
		static_assert( std::is_nothrow_assignable_v<o##P_##SUFFIX_## _t,unsigned short>); \
		static_assert( std::is_nothrow_assignable_v<o##P_##SUFFIX_## _t,unsigned int>); \
		static_assert( std::is_nothrow_assignable_v<o##P_##SUFFIX_## _t,unsigned long>); \
		static_assert( std::is_nothrow_assignable_v<o##P_##SUFFIX_## _t,unsigned long long>); \
		static_assert(!std::is_assignable_v<o##P_##SUFFIX_## _t,Int<std::uintmax_t{(MAX_)+1u},IntKind::fast >>, "no narrowing assignment"); \
		static_assert(!std::is_assignable_v<o##P_##SUFFIX_## _t,Int<std::uintmax_t{(MAX_)+1u},IntKind::small>>, "no narrowing assignment"); \
		static_assert(!std::is_assignable_v<o##P_##SUFFIX_##s_t,Int<std::uintmax_t{(MAX_)+1u},IntKind::fast >>, "no narrowing assignment"); \
		static_assert(!std::is_assignable_v<o##P_##SUFFIX_##s_t,Int<std::uintmax_t{(MAX_)+1u},IntKind::small>>, "no narrowing assignment");

		DEFINE_OX_TYPES(1, i,  static_cast<std::uintmax_t>(O))
		DEFINE_OX_TYPES(2, i,  static_cast<std::uintmax_t>(O*O))
		DEFINE_OX_TYPES(3, i,  static_cast<std::uintmax_t>(O*O*O))
		DEFINE_OX_TYPES(4, i,  static_cast<std::uintmax_t>(O*O*O*O))
		DEFINE_OX_TYPES(1, x,  (O) -1u)
		DEFINE_OX_TYPES(2, x,  (O*O) -1u)
		DEFINE_OX_TYPES(3, x,  (O*O*O) -1u)
		DEFINE_OX_TYPES(4, x,  (O*O*O*O) -1u)
		DEFINE_OX_TYPES(2, x1, (O*O) - (O))
		DEFINE_OX_TYPES(3, x1, (O*O*O) - (O))
		DEFINE_OX_TYPES(4, x1, (O*O*O*O) - (O))
		DEFINE_OX_TYPES(3, x2, (O*O*O) - (O*O))
		DEFINE_OX_TYPES(4, x2, (O*O*O*O) - (O*O))
		#undef DEFINE_OX_TYPES

		static constexpr Int<static_cast<uintmax_t>(O      ), IntKind::fixed> O1 {};
		static constexpr Int<static_cast<uintmax_t>(O*O    ), IntKind::fixed> O2 {};
		static constexpr Int<static_cast<uintmax_t>(O*O*O  ), IntKind::fixed> O3 {};
		static constexpr Int<static_cast<uintmax_t>(O*O*O*O), IntKind::fixed> O4 {};
	};


	#define OKIIDOKU_MONO_INT_TS_TYPEDEFS \
		using T       [[maybe_unused]] = Ints<O>;    \
		using o1x_t   [[maybe_unused]] = T::o1x_t;   \
		using o1i_t   [[maybe_unused]] = T::o1i_t;   \
		using o2x1_t  [[maybe_unused]] = T::o2x1_t;  \
		using o2x_t   [[maybe_unused]] = T::o2x_t;   \
		using o2i_t   [[maybe_unused]] = T::o2i_t;   \
		using o3x2_t  [[maybe_unused]] = T::o3x2_t;  \
		using o3x1_t  [[maybe_unused]] = T::o3x1_t;  \
		using o3x_t   [[maybe_unused]] = T::o3x_t;   \
		using o3i_t   [[maybe_unused]] = T::o3i_t;   \
		using o4x2_t  [[maybe_unused]] = T::o4x2_t;  \
		using o4x1_t  [[maybe_unused]] = T::o4x1_t;  \
		using o4x_t   [[maybe_unused]] = T::o4x_t;   \
		using o4i_t   [[maybe_unused]] = T::o4i_t;   \
		using o1xs_t  [[maybe_unused]] = T::o1xs_t;  \
		using o1is_t  [[maybe_unused]] = T::o1is_t;  \
		using o2x1s_t [[maybe_unused]] = T::o2x1s_t; \
		using o2xs_t  [[maybe_unused]] = T::o2xs_t;  \
		using o2is_t  [[maybe_unused]] = T::o2is_t;  \
		using o3x2s_t [[maybe_unused]] = T::o3x2s_t; \
		using o3x1s_t [[maybe_unused]] = T::o3x1s_t; \
		using o3xs_t  [[maybe_unused]] = T::o3xs_t;  \
		using o3is_t  [[maybe_unused]] = T::o3is_t;  \
		using o4x2s_t [[maybe_unused]] = T::o4x2s_t; \
		using o4x1s_t [[maybe_unused]] = T::o4x1s_t; \
		using o4xs_t  [[maybe_unused]] = T::o4xs_t;  \
		using o4is_t  [[maybe_unused]] = T::o4is_t;
		// using rmi_t [[maybe_unused]] = T::o4xs_t;


	/** the value O2 (maximum value) represents an empty grid cell. */
	template<Order O>
	using grid_sym_t = Ints<O>::o2is_t;


	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_row(const typename Ints<O>::o4x_t rmi) noexcept {
		using T = Ints<O>;
		return rmi / T::O2;
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_col(const typename Ints<O>::o4x_t rmi) noexcept {
		using T = Ints<O>;
		return rmi % T::O2;
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t row_col_to_box(
		const typename Ints<O>::o2x_t row,
		const typename Ints<O>::o2x_t col
	) noexcept {
		using T = Ints<O>;
		return ((row / T::O1) * T::O1) + (col / T::O1);
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t row_col_to_box_cell(
		const typename Ints<O>::o2x_t row,
		const typename Ints<O>::o2x_t col
	) noexcept {
		using T = Ints<O>;
		return ((row % T::O1) * T::O1) + (col % T::O1);
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_box(const typename Ints<O>::o4x_t rmi) noexcept {
		return row_col_to_box<O>(rmi_to_row<O>(rmi), rmi_to_col<O>(rmi));
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_box_cell(const typename Ints<O>::o4x_t rmi) noexcept {
		using T = Ints<O>;
		const auto boxrow {(rmi/T::O2) % T::O1};
		const auto boxcol {rmi % T::O1};
		return (T::O1*boxrow) + boxcol;
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_house(const HouseType house_type, const typename Ints<O>::o4x_t rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT(rmi < T::O4);
		switch (house_type) {
			using enum HouseType;
			case row: return rmi_to_row<O>(rmi);
			case col: return rmi_to_col<O>(rmi);
			case box: return rmi_to_box<O>(rmi);
		}
		OKIIDOKU_UNREACHABLE;
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t row_col_to_rmi(
		const typename Ints<O>::o2x_t row,
		const typename Ints<O>::o2x_t col
	) noexcept {
		using T = Ints<O>;
		return (T::O2 * row) + col;
	}

	/**
	\param box row-major index of a box in a grid.
	\param box_cell row-major index of a cell within `box`.
	\pre `box` and `box_cell` are in `[0, O2)`. */
	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t box_cell_to_rmi(
		const typename Ints<O>::o2x_t box,
		const typename Ints<O>::o2x_t box_cell
	) noexcept {
		using T = Ints<O>;
		const auto row {((box/T::O1)*T::O1) + (box_cell/T::O1)};
		const auto col {((box%T::O1)*T::O1) + (box_cell%T::O1)};
		return row_col_to_rmi<O>(row, col);
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t house_cell_to_rmi(
		const HouseType house_type,
		const typename Ints<O>::o2x_t house,
		const typename Ints<O>::o2x_t house_cell
	) noexcept {
		switch (house_type) {
			using enum HouseType;
			case row: return row_col_to_rmi<O>(house, house_cell);
			case col: return row_col_to_rmi<O>(house_cell, house);
			case box: return box_cell_to_rmi<O>(house, house_cell);
		}
		OKIIDOKU_UNREACHABLE;
	}

	template<Order O> [[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t chute_cell_to_rmi(
		const LineType line_type,
		const typename Ints<O>::o1x_t chute,
		const typename Ints<O>::o3x_t chute_cell
	) noexcept {
		using T = Ints<O>;
		switch (line_type) {
			using enum LineType;
			case row: return (T::O3*chute) + chute_cell;
			case col: return ((T::O1*chute)+((chute_cell%T::O2)*T::O2)+(chute_cell/T::O2));
		}
		OKIIDOKU_UNREACHABLE;
	}
}


/**
this namespace generally mirrors `::okiidoku::mono`, except it wraps the per-grid-order
templates into algebraic types (tagged unions) for convenience for applications which
work with grid orders selected at runtime. it's named `visitor` because it uses the
visitor pattern to dispatch to the right template instantiations of function calls.
*/
namespace okiidoku::visitor {

	// using Ints = mono::Ints<largest_compiled_order>;

	namespace ints {
		using o1x_t  = mono::Ints<largest_compiled_order>::o1x_t::val_t;
		using o1i_t  = mono::Ints<largest_compiled_order>::o1i_t::val_t;
		using o2x_t  = mono::Ints<largest_compiled_order>::o2x_t::val_t;
		using o2i_t  = mono::Ints<largest_compiled_order>::o2i_t::val_t;
		using o2xs_t = mono::Ints<largest_compiled_order>::o2xs_t::val_t;
		using o2is_t = mono::Ints<largest_compiled_order>::o2is_t::val_t;
		using o4x_t  = mono::Ints<largest_compiled_order>::o4x_t::val_t;
		using o4i_t  = mono::Ints<largest_compiled_order>::o4i_t::val_t;
		using o4xs_t = mono::Ints<largest_compiled_order>::o4xs_t::val_t;
		using o4is_t = mono::Ints<largest_compiled_order>::o4is_t::val_t;
	}
	using grid_sym_t = mono::grid_sym_t<largest_compiled_order>;
}
#endif