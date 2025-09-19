// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__INTS
#define HPP_OKIIDOKU__INTS

#include <okiidoku/order.hpp> // Order, largest_compiled_order

// #include <iosfwd>
#include <array>
#include <bit>         // bit_width
#include <utility>     // forward, to_underlying
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral
#include <compare>     //
#include <cstdint>     // uint_...

namespace emscripten::internal { template<typename T, typename> struct BindingType; }

/** top-level namespace for the okiidoku project */
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
	struct HouseTypeMap {
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& at(this Self&& self, const HouseType key) noexcept { return std::forward<Self>(self).arr_[std::to_underlying(key)]; }
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& get_underlying_arr(this Self&& self) noexcept { return std::forward<Self>(self).arr_; }
	private:
		std::array<V, house_types.size()> arr_;
	};

	enum class LineType : unsigned char {
		row, col,
	};
	inline constexpr auto line_types {std::to_array<LineType>({
		LineType::row,
		LineType::col,
	})};
	template<typename V> requires(!std::is_reference_v<V>)
	struct LineTypeMap {
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& at(this Self&& self, const LineType key) noexcept { return std::forward<Self>(self).arr_[std::to_underlying(key)]; }
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& get_underlying_arr(this Self&& self) noexcept { return std::forward<Self>(self).arr_; }
	private:
		std::array<V, line_types.size()> arr_;
	};

	enum class BoxOrLine : unsigned char {
		box, line,
	};

	enum class CellOrSym : unsigned char {
		cell, sym,
	};
}


/**
this namespace is for the per-grid-order definitions of okiidoku library features.
most things are templatized based on grid order to optimize memory usage (at the
expense of generating code for each grid order seleted by the user). to choose what
grid orders to generate, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header)
for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).
*/
namespace okiidoku::mono {

	/// \cond detail
	namespace detail {

		template<int W> requires(W <= 64)
		/// fast unsigned integer type that can fit at least `N` bits.
		using uint_fast_for_width_t = typename
			std::conditional_t<(W <=  8), std::uint_fast8_t ,
			std::conditional_t<(W <= 16), std::uint_fast16_t,
			std::conditional_t<(W <= 32), std::uint_fast32_t,
			std::conditional_t<(W <= 64), std::uint_fast64_t,
			// std::conditional_t<(W <= 128u), __uint128_t, // currently unused. Note: won't work with MSVC
			void
		>>>>;

		template<int W> requires(W <= 64)
		/// smallest unsigned integer type that can fit at least `N` bits.
		using uint_small_for_width_t = typename
			std::conditional_t<(W <=  8), std::uint_least8_t ,
			std::conditional_t<(W <= 16), std::uint_least16_t,
			std::conditional_t<(W <= 32), std::uint_least32_t,
			std::conditional_t<(W <= 64), std::uint_least64_t,
			// std::conditional_t<(W <= 128u), __uint128_t,
			void
		>>>>;

		template<std::uintmax_t M>
		/// fast unsigned integer type that can fit max value `M`.
		using uint_fast_for_max_t = typename
			std::conditional_t<(M <= std::numeric_limits<std::uint8_t >::max()), std::uint_fast8_t ,
			std::conditional_t<(M <= std::numeric_limits<std::uint16_t>::max()), std::uint_fast16_t,
			std::conditional_t<(M <= std::numeric_limits<std::uint32_t>::max()), std::uint_fast32_t,
			std::conditional_t<(M <= std::numeric_limits<std::uint64_t>::max()), std::uint_fast64_t,
			void
		>>>>;

		template<int M>
		/// smallest unsigned integer type that can fit max value `M`.
		using uint_small_for_max_t = typename
			std::conditional_t<(M <= std::numeric_limits<std::uint8_t >::max()), std::uint_least8_t ,
			std::conditional_t<(M <= std::numeric_limits<std::uint16_t>::max()), std::uint_least16_t,
			std::conditional_t<(M <= std::numeric_limits<std::uint32_t>::max()), std::uint_least32_t,
			std::conditional_t<(M <= std::numeric_limits<std::uint64_t>::max()), std::uint_least64_t,
			void
		>>>>;

		template<int M>
		/// fast signed integer type that can fit max value `M`.
		using int_fast_for_max_t = typename
			std::conditional_t<(M <= std::numeric_limits<std::int8_t >::max()), std::int_fast8_t ,
			std::conditional_t<(M <= std::numeric_limits<std::int16_t>::max()), std::int_fast16_t,
			std::conditional_t<(M <= std::numeric_limits<std::int32_t>::max()), std::int_fast32_t,
			std::conditional_t<(M <= std::numeric_limits<std::int64_t>::max()), std::int_fast64_t,
			void
		>>>>;

		template<int M>
		/// smallest signed integer type that can fit max value `M`.
		using int_small_for_max_t = typename
			std::conditional_t<(M <= std::numeric_limits<std::int8_t >::max()), std::int_least8_t ,
			std::conditional_t<(M <= std::numeric_limits<std::int16_t>::max()), std::int_least16_t,
			std::conditional_t<(M <= std::numeric_limits<std::int32_t>::max()), std::int_least32_t,
			std::conditional_t<(M <= std::numeric_limits<std::int64_t>::max()), std::int_least64_t,
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
	/// \endcond


	enum class IntKind : unsigned char {
		small,
		fast,
	};
	static constexpr IntKind pick_int_op_result_kind(const IntKind k1, const IntKind k2) noexcept {
		return (k1 == IntKind::fast || k2 == IntKind::fast) ? IntKind::fast : IntKind::small;
	}

	/** integer wrapper class with inclusive upper bound.
	iterable with the upper bound as an excluded sentinel.
	*/
	template<std::uintmax_t max_, IntKind kind_ = IntKind::fast>
	class Int {
		template<std::uintmax_t M2, IntKind K2> friend class Int;
		friend struct emscripten::internal::BindingType<Int, std::true_type>;
		using max_t = std::uintmax_t;
	public:
		static constexpr max_t max {max_};
		static constexpr IntKind kind {kind_};
		using val_t = std::conditional_t<kind == IntKind::small,
			detail::uint_small_for_width_t<std::bit_width(max)>,
			detail::uint_fast_for_width_t<std::bit_width(max)>
		>;
		using difference_type = detail::int_fast_for_max_t<max>;
	private:
		val_t val_ {0u};
		static_assert(std::is_unsigned_v<val_t>);
		static_assert(std::numeric_limits<val_t>::max() >= max, "has enough capacity");
	public:
		[[gnu::always_inline]] constexpr void check() const noexcept {
			OKIIDOKU_CONTRACT_USE(val_ <= max);
		}

		constexpr Int() noexcept = default;
		// constexpr Int(const Int& other) noexcept = default;

		/** \pre `0 <= in <= max` */
		template<class T_in> requires(std::integral<T_in>)
		constexpr Int(T_in in) noexcept: val_{static_cast<val_t>(in)} {
			if constexpr (std::is_signed_v<T_in>) {
				OKIIDOKU_CONTRACT_USE(in >= 0);
			}
			OKIIDOKU_CONTRACT_USE(static_cast<max_t>(in) <= max);
			check();
		}

		// define implicit conversion from narrower `Int`.
		template<max_t M_from, IntKind K_from> requires(M_from <= max)
		constexpr Int(const Int<M_from,K_from>& other) noexcept: Int{other.val_} { check(); }

		// define explicit conversion from `Int` "wider" by one,
		// to allow cast to sentinel-exclusive-bound `Int` when iterating.
		template<IntKind K_from>
		explicit constexpr Int(const Int<max+1u,K_from>& other) noexcept: Int{other.val_} { check(); }

		template<max_t M_from, IntKind K_from>
		static constexpr Int unchecked_from(const Int<M_from,K_from>& other) noexcept {
			OKIIDOKU_CONTRACT_USE(other.val_ <= max);
			return Int{other.val_};
		}

		// define implicit conversion to a fast, builtin, unsigned integer type with enough capacity.
		[[gnu::pure]] constexpr operator detail::uint_fast_for_width_t<std::bit_width(max)>() const noexcept { check(); return static_cast<detail::uint_small_for_width_t<std::bit_width(max)>>(val_); }
		// template<class T> requires(std::unsigned_integral<T> && std::numeric_limits<T>::max() >= max)
		// constexpr operator T() const noexcept { check(); return val_; }
		// friend std::ostream& operator<<(std::ostream& os, const Int& obj) { return os << static_cast<max_t>(i.val_); }
		[[nodiscard, gnu::pure]] constexpr val_t val() const noexcept { check(); return val_; }
		[[nodiscard, gnu::pure]] constexpr Int<max, IntKind::fast>  as_fast()  const noexcept { return Int<max, IntKind::fast>{val_}; }
		[[nodiscard, gnu::pure]] constexpr Int<max, IntKind::small> as_small() const noexcept { return Int<max, IntKind::small>{val_}; }
		constexpr auto& operator++(   ) & noexcept { OKIIDOKU_CONTRACT_USE(val_ < max); ++val_; check(); return *this; }
		constexpr auto  operator++(int) & noexcept { const auto old {*this}; operator++(); return old; }
		constexpr auto& operator--(   ) & noexcept { OKIIDOKU_CONTRACT_USE(val_ > val_t{0u}); --val_; check(); return *this; }

		// TODO move this into dedicated iter_x class?
		[[nodiscard, gnu::pure]] constexpr auto  begin() const noexcept { check(); return Int{0u}; }
		[[nodiscard, gnu::pure]] constexpr auto  end()   const noexcept { check(); return *this; }
		[[nodiscard, gnu::pure]] constexpr auto  next()  const noexcept { check(); return Int{val_+1u}; }
		template<max_t M, IntKind K> requires(M > 0u) [[gnu::pure]] constexpr
		friend Int<max-1u,kind> operator*(const Int<M,K>& it) noexcept {
			it.check(); OKIIDOKU_CONTRACT_USE(it.val_ < M);
			return Int<M-1u,K>{it};
		}

		/// \pre `val_ + other.val_ <= max`
		template<max_t MR, IntKind kind_2>
		constexpr Int& operator+=(const Int<MR, kind_2>& other) & noexcept requires(MR <= max) {
			check(); other.check();
			OKIIDOKU_CONTRACT_USE(max_t{other.val_} <= max);
			OKIIDOKU_CONTRACT_USE(max_t{val_} <= max_t{max - max_t{other.val_}}); // i.e. `val_ + other.val_ <= max`
			OKIIDOKU_CONTRACT_USE(max_t{val_} + other.val_ <= max);
			val_ += other.val_;
			check();
			return *this;
		}

		/// \pre `val_ - other.val_ >= 0`
		template<max_t MR, IntKind kind_2>
		constexpr Int& operator-=(const Int<MR, kind_2>& other) & noexcept requires(MR <= max) {
			check(); other.check();
			OKIIDOKU_CONTRACT_USE(other.val_ <= val_);
			val_ -= other.val_;
			check();
			return *this;
		}

		/// \pre `other.val_ != 0`
		template<max_t MR, IntKind kind_2> requires(MR > 0u)
		constexpr Int& operator/=(const Int<MR, kind_2>& other) & noexcept requires(MR <= max) {
			check(); other.check();
			OKIIDOKU_CONTRACT_USE(other.val_ != val_t{0});
			val_ /= other.val_;
			check();
			return *this;
		}

		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend std::strong_ordering operator<=>(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator==(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator!=(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator<=(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator< (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator>=(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend bool operator> (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator+ (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> requires(ML >= MR) constexpr friend auto operator-(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> constexpr friend auto operator* (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> requires(MR >  0u) constexpr friend auto operator/ (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t ML, IntKind KL, max_t MR, IntKind KR> requires(MR >  0u) constexpr friend auto operator%(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept;
		template<max_t MR, IntKind KR> requires(MR > 0u) constexpr friend auto operator%(const std::unsigned_integral auto lhs, const Int<MR,KR>& rhs) noexcept;
	};


	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr std::strong_ordering operator<=>(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ <=> rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator== (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ == rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator!= (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ != rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator<= (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ <= rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator<  (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ <  rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator>= (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ >= rhs.val_; }
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> [[gnu::pure]] constexpr bool operator>  (const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept { lhs.check(); rhs.check(); return lhs.val_ >  rhs.val_; }

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::pure]] constexpr auto operator+(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept {
		lhs.check(); rhs.check();
		constexpr auto m_out {ML + MR};
		constexpr auto k_out {pick_int_op_result_kind(KL,KR)};
		return Int<m_out,k_out>{lhs.val_ + rhs.val_};
	}

	/**
	\pre `lhs >= rhs`
	\note uses size of `lhs`. doesn't assume minimum value of `rhs`. */
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> requires(ML >= MR)
	[[gnu::pure]] constexpr auto operator-(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept {
		lhs.check(); rhs.check();
		constexpr auto m_out {ML};
		constexpr auto k_out {pick_int_op_result_kind(KL,KR)};
		return Int<m_out,k_out>{lhs.val_ - rhs.val_};
	}

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR>
	[[gnu::pure]] constexpr auto operator*(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept {
		lhs.check(); rhs.check();
		constexpr auto m_out {ML * MR};
		constexpr auto k_out {pick_int_op_result_kind(KL,KR)};
		return Int<m_out,k_out>{lhs.val_ * rhs.val_};
	}

	/// \pre `rhs` is its type's maximum value.
	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> requires(MR > 0u)
	[[gnu::pure]] constexpr auto operator/(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept {
		lhs.check(); rhs.check();
		OKIIDOKU_CONTRACT_USE(rhs.val_ == MR);
		constexpr auto m_out {ML / MR};
		constexpr auto k_out {pick_int_op_result_kind(KL,KR)};
		return Int<m_out,k_out>{lhs.val_ / rhs.val_};
	}

	template<std::uintmax_t ML, IntKind KL, std::uintmax_t MR, IntKind KR> requires(MR > 0u)
	[[gnu::pure]] constexpr auto operator%(const Int<ML,KL>& lhs, const Int<MR,KR>& rhs) noexcept {
		lhs.check(); rhs.check();
		constexpr auto m_out {MR - 1u};
		constexpr auto k_out {pick_int_op_result_kind(KL,KR)};
		const Int<m_out,k_out> ret {lhs.val_ % rhs.val_};
		OKIIDOKU_CONTRACT_USE(ret.val_ < rhs.val_);
		OKIIDOKU_CONTRACT_USE(ret < rhs);
		return ret;
	}
	template<std::uintmax_t MR, IntKind KR> requires(MR > 0u)
	[[gnu::pure]] constexpr auto operator%(const std::unsigned_integral auto lhs, const Int<MR,KR>& rhs) noexcept {
		rhs.check();
		const Int<MR-1,KR> ret {lhs % rhs.val_};
		OKIIDOKU_CONTRACT_USE(ret.val_ < rhs.val_);
		OKIIDOKU_CONTRACT_USE(ret < rhs);
		return ret;
	}


	/** a class acting as a namespace of sized int types and constants.
	\details do `using T = Ints<O>;`.
	\note when printing things, make sure to cast to int, since byte-like types will be
		interpreted as characters.
	\internal the producer functions are a step in a good direction, but I'm guessing
		it would be better to use user-defined integral types that enforce bounds in the
		their constructors, define conversions to unsigned integer types, delete those to
		signed integral types (if that's a thing), and define helpers for operations that
		guarantee size of output like modulo. */
	template<Order O> requires(is_order_compiled(O))
	struct Ints {
		Ints() = delete;

		#define DEFINE_OX_TYPES(P_, SUFFIX_, MAX_) \
		using o##P_##SUFFIX_##_t  = Int<(MAX_), IntKind::fast >; \
		using o##P_##SUFFIX_##s_t = Int<(MAX_), IntKind::small>; \
		\
		static_assert(sizeof(o##P_##SUFFIX_## _t) == sizeof(typename o##P_##SUFFIX_## _t::val_t)); \
		static_assert(sizeof(o##P_##SUFFIX_##s_t) == sizeof(typename o##P_##SUFFIX_##s_t::val_t)); \
		static_assert( std::is_trivially_copyable_v<o##P_##SUFFIX_## _t>); \
		static_assert( std::is_trivially_copyable_v<o##P_##SUFFIX_##s_t>); \
		static_assert( std::is_nothrow_convertible_v<o##P_##SUFFIX_## _t, o##P_##SUFFIX_##s_t>, "can convert fast to small"); \
		static_assert( std::is_nothrow_convertible_v<o##P_##SUFFIX_##s_t, o##P_##SUFFIX_## _t>, "can convert small to fast"); \
		static_assert( std::is_nothrow_convertible_v<Int<(MAX_)-1u>,o##P_##SUFFIX_## _t>, "can convert from narrower Int"); \
		static_assert( std::is_nothrow_convertible_v<Int<(MAX_)-1u>,o##P_##SUFFIX_##s_t>, "can convert from narrower Int"); \
		static_assert(!std::is_nothrow_convertible_v<Int<(MAX_)+1u>,o##P_##SUFFIX_## _t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<(MAX_)+1u>,o##P_##SUFFIX_##s_t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<(MAX_)+1u,IntKind::small>,o##P_##SUFFIX_## _t>, "no implicit conversion from sentinel type"); \
		static_assert(!std::is_nothrow_convertible_v<Int<(MAX_)+1u,IntKind::small>,o##P_##SUFFIX_##s_t>, "no implicit conversion from sentinel type"); \
		static_assert( std::is_nothrow_constructible_v<o##P_##SUFFIX_## _t,Int<(MAX_)+1u>>, "explicit conversion from sentinel type"); \
		static_assert( std::is_nothrow_constructible_v<o##P_##SUFFIX_##s_t,Int<(MAX_)+1u>>, "explicit conversion from sentinel type"); \
		static_assert( std::is_nothrow_constructible_v<o##P_##SUFFIX_## _t,Int<(MAX_)+1u,IntKind::small>>, "explicit conversion from sentinel type"); \
		static_assert( std::is_nothrow_constructible_v<o##P_##SUFFIX_##s_t,Int<(MAX_)+1u,IntKind::small>>, "explicit conversion from sentinel type");

		DEFINE_OX_TYPES(1, i,  O)
		DEFINE_OX_TYPES(2, i,  O*O)
		DEFINE_OX_TYPES(3, i,  O*O*O)
		DEFINE_OX_TYPES(4, i,  O*O*O*O)
		DEFINE_OX_TYPES(1, x,  (O) -1u)
		DEFINE_OX_TYPES(2, x,  (O*O) -1u)
		DEFINE_OX_TYPES(3, x,  (O*O*O) -1u)
		DEFINE_OX_TYPES(4, x,  (O*O*O*O) -1u)
		DEFINE_OX_TYPES(2, x1 ,(O*O) -O)
		DEFINE_OX_TYPES(3, x1 ,(O*O*O) -O)
		DEFINE_OX_TYPES(4, x1 ,(O*O*O*O) -O)
		DEFINE_OX_TYPES(3, x2 ,(O*O*O) -(O*O))
		DEFINE_OX_TYPES(4, x2 ,(O*O*O*O) -(O*O))
		#undef DEFINE_OX_TYPES

		static constexpr o1i_t O1 {o1i_t::max};
		static constexpr o2i_t O2 {o2i_t::max};
		static constexpr o3i_t O3 {o3i_t::max};
		static constexpr o4i_t O4 {o4i_t::max};
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


	template<Order O>
	using grid_val_t = Ints<O>::o2is_t;


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
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
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
this namespace generally mirrors `okiidoku::mono`, except it wraps the per-grid-order
templates into algebraic types (tagged unions) for convience for applications which
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
	using grid_val_t = mono::grid_val_t<largest_compiled_order>;
}
#endif