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
#include <cstdint>     // uint_...

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

		template<int W>
		/// fast unsigned integer type that can fit at least `N` bits.
		using uint_fast_for_width_t = typename
			std::conditional_t<(W <=   8), std::uint_fast8_t,
			std::conditional_t<(W <=  16), std::uint_fast16_t,
			std::conditional_t<(W <=  32), std::uint_fast32_t,
			std::conditional_t<(W <=  64), std::uint_fast64_t,
			// std::conditional_t<(W <= 128u), __uint128_t, // currently unused. Note: won't work with MSVC
			void
		>>>>;

		template<int W>
		/// smallest unsigned integer type that can fit at least `N` bits.
		using uint_small_for_width_t = typename
			std::conditional_t<(W <=   8), std::uint_least8_t,
			std::conditional_t<(W <=  16), std::uint_least16_t,
			std::conditional_t<(W <=  32), std::uint_least32_t,
			std::conditional_t<(W <=  64), std::uint_least64_t,
			// std::conditional_t<(W <= 128u), __uint128_t,
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

		enum class IntKind : unsigned char {
			small,
			fast,
		};

		template<std::uintmax_t max_, IntKind int_kind_ = IntKind::fast>
		class Int {
		public:
			using max_t = std::uintmax_t;
			static constexpr max_t max {max_};
			static constexpr IntKind int_kind {int_kind_};
			using i_t = std::conditional_t<int_kind == IntKind::small,
				uint_small_for_width_t<std::bit_width(max)>,
				uint_fast_for_width_t<std::bit_width(max)>
			>;
		private:
			template<max_t max_2, IntKind int_kind_2> friend class Int;
			static_assert(std::is_unsigned_v<i_t>, "underlying type is unsigned");
			static_assert(std::numeric_limits<i_t>::max() >= max, "underlying type has enough capacity");
			i_t val_; // TODO does adding initializer to zero impact performance?
			[[gnu::always_inline]] constexpr void check() const noexcept {
				OKIIDOKU_CONTRACT_USE(val_ <= max);
			}
		public:
			constexpr Int() noexcept {} // TODO what's the difference with using the default initializer?
			/** \pre `0 <= in <= max` */
			template<class T_in> requires(std::integral<T_in> && std::numeric_limits<T_in>::max() <= std::numeric_limits<max_t>::max())
			constexpr Int(T_in in) noexcept: val_{static_cast<i_t>(in)} {
				if constexpr (std::is_signed_v<T_in>) {
					OKIIDOKU_CONTRACT_USE(in >= T_in{0});
				}
				OKIIDOKU_CONTRACT_USE(max_t(in) <= max);
				check();
			}
			template<max_t max_2, IntKind int_kind_2> requires(max_2 <= max+1u) // (leeway for range sentinel)
			constexpr Int(Int<max_2, int_kind_2> other) noexcept: Int{other.val_} { check(); }
		public:
			constexpr operator uint_fast_for_width_t<std::bit_width(max)>() const noexcept { check(); return static_cast<uint_small_for_width_t<std::bit_width(max)>>(val_); }
			// template<class T> requires(std::unsigned_integral<T> && std::numeric_limits<T>::max() >= max)
			// constexpr operator T() const noexcept { check(); return val_; }
			// friend std::ostream& operator<<(std::ostream& os, const Int& obj) { return os << static_cast<max_t>(i.val_); }
			[[nodiscard, gnu::pure]] constexpr i_t get_underlying() const noexcept { check(); return val_; }
			[[nodiscard, gnu::pure]] constexpr Int<max, IntKind::fast>  as_fast()  const noexcept { return Int<max, IntKind::fast>{val_}; }
			[[nodiscard, gnu::pure]] constexpr Int<max, IntKind::small> as_small() const noexcept { return Int<max, IntKind::small>{val_}; }
			constexpr auto& operator++()    & noexcept { OKIIDOKU_CONTRACT_USE(val_ < max); ++val_; check(); return *this; }
			constexpr auto  operator++(int) & noexcept { const auto old {*this}; operator++(); return old; }
			constexpr auto& operator--()    & noexcept { OKIIDOKU_CONTRACT_USE(val_ > i_t{0}); --val_; check(); return *this; }

			// TODO move this into dedicate iter_x class?
			[[nodiscard, gnu::pure]] constexpr auto  begin() const noexcept { check(); return Int{0u}; }
			[[nodiscard, gnu::pure]] constexpr auto  end()   const noexcept { check(); return *this; }
			[[nodiscard, gnu::pure]] constexpr auto  next()  const noexcept { check(); return Int{val_+1u}; }
			constexpr Int<max-1u,int_kind> operator*() const noexcept { check(); OKIIDOKU_CONTRACT_USE(val_ < max); return *this; }

			/// \pre `val_ + other.val_ <= max`
			template<max_t max_2, IntKind int_kind_2>
			constexpr Int& operator+=(const Int<max_2, int_kind_2>& other) & noexcept requires(max_2 <= max) {
				check();
				OKIIDOKU_CONTRACT_USE(max_t{other.val_} <= max);
				OKIIDOKU_CONTRACT_USE(max_t{val_} <= max_t{max - max_t{other.val_}}); // i.e. `val_ + other.val_ <= max`
				OKIIDOKU_CONTRACT_USE(val_ + other.val_ <= max);
				val_ += other.val_;
				check();
				return *this;
			}

			/// \pre `val_ - other.val_ >= 0`
			template<max_t max_2, IntKind int_kind_2>
			constexpr Int& operator-=(const Int<max_2, int_kind_2>& other) & noexcept requires(max_2 <= max) {
				check();
				OKIIDOKU_CONTRACT_USE(other.val_ <= val_);
				val_ -= other.val_;
				check();
				return *this;
			}

			/// \pre `other.val_ != 0`
			template<max_t max_2, IntKind int_kind_2>
			constexpr Int& operator/=(const Int<max_2, int_kind_2>& other) & noexcept requires(max_2 <= max) {
				check();
				OKIIDOKU_CONTRACT_USE(other.val_ != i_t{0});
				val_ /= other.val_;
				check();
				return *this;
			}

			template<max_t m1, IntKind k1, max_t m2, IntKind k2> constexpr friend auto operator+(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept;
			template<max_t m1, IntKind k1, max_t m2, IntKind k2> constexpr friend auto operator*(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept;
			template<max_t m1, IntKind k1, max_t m2, IntKind k2> constexpr friend auto operator/(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept;
			template<max_t m1, IntKind k1, max_t m2, IntKind k2> constexpr friend auto operator%(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept;
		};

		template<std::uintmax_t m1, IntKind k1, std::uintmax_t m2, IntKind k2>
		[[gnu::pure]] constexpr auto operator+(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept {
			constexpr auto m_out = m1 + m2;
			constexpr auto k_out = (k1 == IntKind::fast || k2 == IntKind::fast) ? IntKind::fast : IntKind::small;
			return Int<m_out, k_out>{lhs.val_ + rhs.val_};
		}

		template<std::uintmax_t m1, IntKind k1, std::uintmax_t m2, IntKind k2>
		[[gnu::pure]] constexpr auto operator*(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept {
			constexpr auto m_out = m1 * m2;
			constexpr auto k_out = (k1 == IntKind::fast || k2 == IntKind::fast) ? IntKind::fast : IntKind::small;
			return Int<m_out, k_out>{lhs.val_ * rhs.val_};
		}

		/// \pre `rhs` is its type's maximum value.
		template<std::uintmax_t m1, IntKind k1, std::uintmax_t m2, IntKind k2>
		[[gnu::pure]] constexpr auto operator/(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept {
			OKIIDOKU_CONTRACT_USE(rhs.val_ == m2);
			constexpr auto m_out = m1 / m2;
			constexpr auto k_out = (k1 == IntKind::fast || k2 == IntKind::fast) ? IntKind::fast : IntKind::small;
			return Int<m_out, k_out>{lhs.val_ / rhs.val_};
		}

		template<std::uintmax_t m1, IntKind k1, std::uintmax_t m2, IntKind k2>
		[[gnu::pure]] constexpr auto operator%(const Int<m1,k1>& lhs, const Int<m2,k2>& rhs) noexcept {
			constexpr auto m_out = m2 - 1u;
			constexpr auto k_out = (k1 == IntKind::fast || k2 == IntKind::fast) ? IntKind::fast : IntKind::small;
			return Int<m_out, k_out>{lhs.val_ % rhs.val_};
		}
	}
	/// \endcond


	template<Order O> requires(is_order_compiled(O))
	/** a class acting as a namespace of sized int types and constants.
	\details do `using T = Ints<O>;`.
	\note when printing things, make sure to cast to int, since byte-like types will be
		interpreted as characters.
	\internal the producer functions are a step in a good direction, but I'm guessing
		it would be better to use user-defined integral types that enforce bounds in the
		their constructors, define conversions to unsigned integer types, delete those to
		signed integral types (if that's a thing), and define helpers for operations that
		guarantee size of output like modulo. */
	struct Ints {
		Ints() = delete;

		#define DEFINE_OX_TYPES(P_, OX_) \
		using o##P_##x_t  = detail::Int<uintmax_t{1}*OX_-1u, detail::IntKind::fast>; \
		using o##P_##i_t  = detail::Int<uintmax_t{1}*OX_   , detail::IntKind::fast>; \
		using o##P_##xs_t = detail::Int<uintmax_t{1}*OX_-1u, detail::IntKind::small>; \
		using o##P_##is_t = detail::Int<uintmax_t{1}*OX_   , detail::IntKind::small>; \
		static constexpr o##P_##i_t O##P_ {std::uintmax_t{1}*OX_}; \
		template<class T_in> requires ((!std::integral<T_in> && std::is_nothrow_convertible_v<T_in, o##P_##x_t >) || std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##P_##x_t  o##P_##x (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##P_##x_t >(in); } \
		template<class T_in> requires ((!std::integral<T_in> && std::is_nothrow_convertible_v<T_in, o##P_##i_t >) || std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##P_##i_t  o##P_##i (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##P_##i_t >(in); } \
		template<class T_in> requires ((!std::integral<T_in> && std::is_nothrow_convertible_v<T_in, o##P_##xs_t>) || std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##P_##xs_t o##P_##xs(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##P_##xs_t>(in); } \
		template<class T_in> requires ((!std::integral<T_in> && std::is_nothrow_convertible_v<T_in, o##P_##is_t>) || std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##P_##is_t o##P_##is(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##P_##is_t>(in); }
		DEFINE_OX_TYPES(1, O)
		DEFINE_OX_TYPES(2, O*O)
		DEFINE_OX_TYPES(3, O*O*O)
		DEFINE_OX_TYPES(4, O*O*O*O)
		#undef DEFINE_OX_TYPES
	};
	// TODO try getting rid of the converter/factory functions

	// defines `Any_oPx_t<O,T>`, `Any_oPi_t<O,T>` concepts.
	#define DEFINE_OX_TYPES(P_, OX_) \
	template<Order O, typename T> concept Any_o##P_##x_t = (!std::integral<T> && std::is_nothrow_convertible_v<T, typename Ints<O>::o##P_##xs_t>) || (std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (std::uintmax_t{1}*OX_-1u)); \
	template<Order O, typename T> concept Any_o##P_##i_t = (!std::integral<T> && std::is_nothrow_convertible_v<T, typename Ints<O>::o##P_##xs_t>) || (std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (std::uintmax_t{1}*OX_   ));
	DEFINE_OX_TYPES(1, O)
	DEFINE_OX_TYPES(2, O*O)
	DEFINE_OX_TYPES(3, O*O*O)
	DEFINE_OX_TYPES(4, O*O*O*O)
	#undef DEFINE_OX_TYPES

	#define OKIIDOKU_MONO_INT_TS_TYPEDEFS \
		using T      [[maybe_unused]] = Ints<O>; \
		using o1x_t  [[maybe_unused]] = T::o1x_t;  \
		using o1i_t  [[maybe_unused]] = T::o1i_t;  \
		using o2x_t  [[maybe_unused]] = T::o2x_t;  \
		using o2i_t  [[maybe_unused]] = T::o2i_t;  \
		using o3x_t  [[maybe_unused]] = T::o3x_t;  \
		using o3i_t  [[maybe_unused]] = T::o3i_t;  \
		using o4x_t  [[maybe_unused]] = T::o4x_t;  \
		using o4i_t  [[maybe_unused]] = T::o4i_t;  \
		using o2xs_t [[maybe_unused]] = T::o2xs_t; \
		using o2is_t [[maybe_unused]] = T::o2is_t; \
		using o3xs_t [[maybe_unused]] = T::o3xs_t; \
		using o3is_t [[maybe_unused]] = T::o3is_t; \
		using o4xs_t [[maybe_unused]] = T::o4xs_t; \
		using o4is_t [[maybe_unused]] = T::o4is_t;
		// using rmi_t [[maybe_unused]] = T::o4xs_t;


	template<Order O>
	using grid_val_t = Ints<O>::o2is_t;


	template<Order O, class T_rmi> requires(Any_o4x_t<O,T_rmi>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_row(const T_rmi rmi) noexcept {
		using T = Ints<O>;
		return T::o2x(T::o4x(rmi) / T::O2);
	}

	template<Order O, class T_rmi> requires(Any_o4x_t<O,T_rmi>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_col(const T_rmi rmi) noexcept {
		using T = Ints<O>;
		return T::o2x(T::o4x(rmi) % T::O2);
	}

	template<Order O>
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t row_col_to_box(const typename Ints<O>::o2i_t row, const typename Ints<O>::o2i_t col) noexcept {
		using T = Ints<O>;
		const typename T::o2x_t box {(((T::o2x(row) / T::O1) * T::O1) + T::o1x(T::o2x(col) / T::O1))};
		return box;
	}

	template<Order O>
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t row_col_to_box_cell(const typename Ints<O>::o2i_t row, const typename Ints<O>::o2i_t col) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(row < T::O2);
		OKIIDOKU_CONTRACT_USE(col < T::O2);
		const typename T::o2x_t box_cell {(((row % T::O1) * T::O1) + (col % O))};
		return box_cell;
	}

	template<Order O, class T_rmi> requires(Any_o4x_t<O,T_rmi>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_box(const T_rmi rmi) noexcept {
		OKIIDOKU_CONTRACT_USE(rmi < Ints<O>::O4);
		return row_col_to_box<O>(rmi_to_row<O>(rmi), rmi_to_col<O>(rmi));
	}

	template<Order O>
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_box_cell(const typename Ints<O>::o4x_t rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
		const auto boxrow {T::o1x(T::o2x(rmi/T::O2) % T::O1)};
		const auto boxcol {T::o1x(rmi % T::O1)};
		const typename T::o2x_t box_cell {((T::O1*boxrow)+boxcol)};
		return box_cell;
	}

	template<Order O, class T_rmi> requires(Any_o4x_t<O,T_rmi>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o2x_t rmi_to_house(const HouseType house_type, const T_rmi rmi) noexcept {
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

	template<Order O, class T_row, class T_col>
	requires(Any_o2x_t<O,T_row> && Any_o2x_t<O,T_col>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t row_col_to_rmi(const T_row row, const T_col col) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(row < T::O2);
		OKIIDOKU_CONTRACT_USE(col < T::O2);
		const auto rmi {T::o4x((T::O2 * T::o2x(row)) + T::o2x(col))};
		return rmi;
	}

	/**
	\param box row-major index of a box in a grid.
	\param box_cell row-major index of a cell within `box`.
	\pre `box` and `box_cell` are in `[0, O2)`. */
	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O,T_house> && Any_o2x_t<O,T_house_cell>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t box_cell_to_rmi(const T_house box, const T_house_cell box_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(box < T::O2);
		OKIIDOKU_CONTRACT_USE(box_cell < T::O2);
		const typename T::o2x_t row {(((T::o2x(box)/T::O1)*T::O1) + (T::o2x(box_cell)/T::O1))};
		const typename T::o2x_t col {(((T::o2x(box)%T::O1)*T::O1) + (T::o2x(box_cell)%T::O1))};
		return row_col_to_rmi<O>(row, col);
	}

	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O,T_house> && Any_o2x_t<O,T_house_cell>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t house_cell_to_rmi(const HouseType house_type, const T_house house, const T_house_cell house_cell) noexcept {
		switch (house_type) {
			using enum HouseType;
			case row: return row_col_to_rmi<O>(house, house_cell);
			case col: return row_col_to_rmi<O>(house_cell, house);
			case box: return box_cell_to_rmi<O>(house, house_cell);
		}
		OKIIDOKU_UNREACHABLE;
	}

	template<Order O, class T_chute, class T_chute_cell>
	requires(Any_o1x_t<O,T_chute> && Any_o3x_t<O,T_chute_cell>)
	[[nodiscard, gnu::const]] constexpr
	Ints<O>::o4x_t chute_cell_to_rmi(const LineType line_type, const T_chute chute, const T_chute_cell chute_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(chute < T::O1);
		OKIIDOKU_CONTRACT_USE(chute_cell < T::O3);
		switch (line_type) {
			using enum LineType;
			case row: return (T::O3*T::o1x(chute)) + T::o3x(chute_cell);
			case col: return ((T::O1*T::o1x(chute))+((T::o3x(chute_cell)%T::O2)*T::O2)+(T::o3x(chute_cell)/T::O2));
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
		using o1x_t  = mono::Ints<largest_compiled_order>::o1x_t::i_t;
		using o1i_t  = mono::Ints<largest_compiled_order>::o1i_t::i_t;
		using o2x_t  = mono::Ints<largest_compiled_order>::o2x_t::i_t;
		using o2i_t  = mono::Ints<largest_compiled_order>::o2i_t::i_t;
		using o2xs_t = mono::Ints<largest_compiled_order>::o2xs_t::i_t;
		using o2is_t = mono::Ints<largest_compiled_order>::o2is_t::i_t;
		using o4x_t  = mono::Ints<largest_compiled_order>::o4x_t::i_t;
		using o4i_t  = mono::Ints<largest_compiled_order>::o4i_t::i_t;
		using o4xs_t = mono::Ints<largest_compiled_order>::o4xs_t::i_t;
		using o4is_t = mono::Ints<largest_compiled_order>::o4is_t::i_t;
	}
	using grid_val_t = mono::grid_val_t<largest_compiled_order>;
}
#endif