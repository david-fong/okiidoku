// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__INTS
#define HPP_OKIIDOKU__INTS

#include <okiidoku/order.hpp> // Order, largest_compiled_order

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
	}
	/// \endcond

	namespace int_ts {
	}

	// defines `oOx_t<O>`, `oOi_t<O>`, `oOxs_t<O>`, `oOis_t<O>` types
	// and `oOx<O>`, `oOi<O>`, `oOxs<O>`, `oOis<O>` type-cast functions
	#define DEFINE_OX_TYPES(O_, OX_) \
	template<Order O, typename T> concept Any_o##O_##x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (std::uintmax_t{1}*OX_-1u); \
	template<Order O, typename T> concept Any_o##O_##i_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (std::uintmax_t{1}*OX_   ); \
	namespace int_ts { \
		template<Order O> using o##O_##x_t  = detail::uint_fast_for_width_t <std::bit_width(std::uintmax_t{1}*OX_-1u)>; \
		template<Order O> using o##O_##i_t  = detail::uint_fast_for_width_t <std::bit_width(std::uintmax_t{1}*OX_   )>; \
		template<Order O> using o##O_##xs_t = detail::uint_small_for_width_t<std::bit_width(std::uintmax_t{1}*OX_-1u)>; \
		template<Order O> using o##O_##is_t = detail::uint_small_for_width_t<std::bit_width(std::uintmax_t{1}*OX_   )>; \
		template<Order O, class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] constexpr o##O_##x_t <O> o##O_##x (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##O_##x_t <O>>(in); } \
		template<Order O, class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] constexpr o##O_##i_t <O> o##O_##i (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##O_##i_t <O>>(in); } \
		template<Order O, class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] constexpr o##O_##xs_t<O> o##O_##xs(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##O_##xs_t<O>>(in); } \
		template<Order O, class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] constexpr o##O_##is_t<O> o##O_##is(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##O_##is_t<O>>(in); } \
	}
	DEFINE_OX_TYPES(1, O)
	DEFINE_OX_TYPES(2, O*O)
	DEFINE_OX_TYPES(3, O*O*O)
	DEFINE_OX_TYPES(4, O*O*O*O)
	#undef DEFINE_OX_TYPES


	// Note: when printing things, make sure to cast to int, since byte-like types will be interpreted as characters.
	template<Order O> requires(is_order_compiled(O))
	struct Ints {
		Ints() = delete;

		#define DEFINE_OX_TYPES(O_, OX_) \
		using o##O_##x_t  = detail::uint_fast_for_width_t <std::bit_width(std::uintmax_t{1}*OX_-1u)>; \
		using o##O_##i_t  = detail::uint_fast_for_width_t <std::bit_width(std::uintmax_t{1}*OX_   )>; \
		using o##O_##xs_t = detail::uint_small_for_width_t<std::bit_width(std::uintmax_t{1}*OX_-1u)>; \
		using o##O_##is_t = detail::uint_small_for_width_t<std::bit_width(std::uintmax_t{1}*OX_   )>; \
		static constexpr o##O_##i_t O##O_ {OX_}; \
		template<class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##O_##x_t  o##O_##x (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##O_##x_t >(in); } \
		template<class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##O_##i_t  o##O_##i (const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##O_##i_t >(in); } \
		template<class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##O_##xs_t o##O_##xs(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <  std::uintmax_t{1}*OX_); return static_cast<o##O_##xs_t>(in); } \
		template<class T_in> requires (std::integral<T_in>) [[gnu::const, gnu::always_inline]] static constexpr o##O_##is_t o##O_##is(const T_in in) noexcept { static_assert(sizeof(std::uintmax_t) >= sizeof(T_in)); OKIIDOKU_CONTRACT_USE(in >= 0); OKIIDOKU_CONTRACT_USE(static_cast<std::uintmax_t>(in) <= std::uintmax_t{1}*OX_); return static_cast<o##O_##is_t>(in); }
		DEFINE_OX_TYPES(1, O)
		DEFINE_OX_TYPES(2, O*O)
		DEFINE_OX_TYPES(3, O*O*O)
		DEFINE_OX_TYPES(4, O*O*O*O)
		#undef DEFINE_OX_TYPES
	};

	// // I wish there were a nicer way to alias a function in a macro body... this is... something...
	/*
	#define OKIIDOKU_MONO_INT_TS_TYPEDEFS \
		using T      [[maybe_unused]] = Ints<O>; \
		using o1x_t  [[maybe_unused]] = int_ts::o1x_t<O>;   [[maybe_unused]] static constexpr auto o1x  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o1x<O>(std::forward<decltype(in)>(in)); }; \
		using o1i_t  [[maybe_unused]] = int_ts::o1i_t<O>;   [[maybe_unused]] static constexpr auto o1i  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o1i<O>(std::forward<decltype(in)>(in)); }; \
		using o2x_t  [[maybe_unused]] = int_ts::o2x_t<O>;   [[maybe_unused]] static constexpr auto o2x  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o2x<O>(std::forward<decltype(in)>(in)); }; \
		using o2i_t  [[maybe_unused]] = int_ts::o2i_t<O>;   [[maybe_unused]] static constexpr auto o2i  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o2i<O>(std::forward<decltype(in)>(in)); }; \
		using o3x_t  [[maybe_unused]] = int_ts::o3x_t<O>;   [[maybe_unused]] static constexpr auto o3x  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o3x<O>(std::forward<decltype(in)>(in)); }; \
		using o3i_t  [[maybe_unused]] = int_ts::o3i_t<O>;   [[maybe_unused]] static constexpr auto o3i  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o3i<O>(std::forward<decltype(in)>(in)); }; \
		using o4x_t  [[maybe_unused]] = int_ts::o4x_t<O>;   [[maybe_unused]] static constexpr auto o4x  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o4x<O>(std::forward<decltype(in)>(in)); }; \
		using o4i_t  [[maybe_unused]] = int_ts::o4i_t<O>;   [[maybe_unused]] static constexpr auto o4i  = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o4i<O>(std::forward<decltype(in)>(in)); }; \
		using o2xs_t [[maybe_unused]] = int_ts::o2xs_t<O>;  [[maybe_unused]] static constexpr auto o2xs = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o2xs<O>(std::forward<decltype(in)>(in)); }; \
		using o2is_t [[maybe_unused]] = int_ts::o2is_t<O>;  [[maybe_unused]] static constexpr auto o2is = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o2is<O>(std::forward<decltype(in)>(in)); }; \
		using o3xs_t [[maybe_unused]] = int_ts::o3xs_t<O>;  [[maybe_unused]] static constexpr auto o3xs = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o3xs<O>(std::forward<decltype(in)>(in)); }; \
		using o3is_t [[maybe_unused]] = int_ts::o3is_t<O>;  [[maybe_unused]] static constexpr auto o3is = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o3is<O>(std::forward<decltype(in)>(in)); }; \
		using o4xs_t [[maybe_unused]] = int_ts::o4xs_t<O>;  [[maybe_unused]] static constexpr auto o4xs = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o4xs<O>(std::forward<decltype(in)>(in)); }; \
		using o4is_t [[maybe_unused]] = int_ts::o4is_t<O>;  [[maybe_unused]] static constexpr auto o4is = [](auto&& in) constexpr [[gnu::const, gnu::always_inline]] { return int_ts::o4is<O>(std::forward<decltype(in)>(in)); }; \
		// using rmi_t [[maybe_unused]] = int_ts::o4xs_t<O>;
	*/

	#define OKIIDOKU_MONO_INT_TS_TYPEDEFS \
		using T      [[maybe_unused]] = Ints<O>; \
		using o1x_t  [[maybe_unused]] = int_ts::o1x_t<O>;  \
		using o1i_t  [[maybe_unused]] = int_ts::o1i_t<O>;  \
		using o2x_t  [[maybe_unused]] = int_ts::o2x_t<O>;  \
		using o2i_t  [[maybe_unused]] = int_ts::o2i_t<O>;  \
		using o3x_t  [[maybe_unused]] = int_ts::o3x_t<O>;  \
		using o3i_t  [[maybe_unused]] = int_ts::o3i_t<O>;  \
		using o4x_t  [[maybe_unused]] = int_ts::o4x_t<O>;  \
		using o4i_t  [[maybe_unused]] = int_ts::o4i_t<O>;  \
		using o2xs_t [[maybe_unused]] = int_ts::o2xs_t<O>; \
		using o2is_t [[maybe_unused]] = int_ts::o2is_t<O>; \
		using o3xs_t [[maybe_unused]] = int_ts::o3xs_t<O>; \
		using o3is_t [[maybe_unused]] = int_ts::o3is_t<O>; \
		using o4xs_t [[maybe_unused]] = int_ts::o4xs_t<O>; \
		using o4is_t [[maybe_unused]] = int_ts::o4is_t<O>;
		// using rmi_t [[maybe_unused]] = int_ts::o4xs_t<O>;


	template<Order O>
	using grid_val_t = int_ts::o2is_t<O>;


	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_row(const int_ts::o4i_t<O> rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
		return T::o2x(rmi / (T::O2));
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_col(const int_ts::o4i_t<O> rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
		return T::o2x(T::o4x(rmi) % T::O2);
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> row_col_to_box(const int_ts::o2i_t<O> row, const int_ts::o2i_t<O> col) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(row < T::O2);
		OKIIDOKU_CONTRACT_USE(col < T::O2);
		const auto box {T::o2x(T::o2x((row / O) * O) + T::o1x(col / O))};
		return box;
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> row_col_to_box_cell(const int_ts::o2i_t<O> row, const int_ts::o2i_t<O> col) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(row < T::O2);
		OKIIDOKU_CONTRACT_USE(col < T::O2);
		const auto box_cell {T::o2x(T::o2x((row % O) * O) + (col % O))};
		return box_cell;
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_box(const int_ts::o4i_t<O> rmi) noexcept {
		OKIIDOKU_CONTRACT_USE(rmi < Ints<O>::O4);
		return row_col_to_box<O>(rmi_to_row<O>(rmi), rmi_to_col<O>(rmi));
	}

	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_box_cell(const int_ts::o4x_t<O> rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
		const auto boxrow {T::o1x(T::o2x(rmi/T::O2) % T::O1)};
		const auto boxcol {T::o1x(rmi % T::O1)};
		const auto box_cell {T::o2x((T::O1*boxrow)+boxcol)};
		return box_cell;
	}

	template<Order O, class T_rmi>
	requires(Any_o4x_t<O, T_rmi>) [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_house(const HouseType house_type, const T_rmi rmi) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(rmi < T::O4);
		switch (house_type) {
			using enum HouseType;
			case row: return int_ts::o2x<O>(rmi_to_row<O>(rmi));
			case col: return int_ts::o2x<O>(rmi_to_col<O>(rmi));
			case box: return int_ts::o2x<O>(rmi_to_box<O>(rmi));
		}
		OKIIDOKU_UNREACHABLE;
	}

	template<Order O, class T_row, class T_col>
	requires(Any_o2x_t<O, T_row> && Any_o2x_t<O, T_col>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> row_col_to_rmi(const T_row row, const T_col col) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(row < T::O2);
		OKIIDOKU_CONTRACT_USE(col < T::O2);
		const auto rmi {T::o4x((T::O2 * row) + col)};
		return rmi;
	}

	/**
	\param box row-major index of a box in a grid.
	\param box_cell row-major index of a cell within `box`.
	\pre `box` and `box_cell` are in `[0, O2)`. */
	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> box_cell_to_rmi(const T_house box, const T_house_cell box_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(box < T::O2);
		OKIIDOKU_CONTRACT_USE(box_cell < T::O2);
		const auto row {T::o4x(((box/T::O1)*T::O1) + (box_cell/T::O1))};
		const auto col {T::o4x(((box%T::O1)*T::O1) + (box_cell%T::O1))};
		return row_col_to_rmi<O>(row, col);
	}

	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> house_cell_to_rmi(const HouseType house_type, const T_house house, const T_house_cell house_cell) noexcept {
		switch (house_type) {
			using enum HouseType;
			case row: return row_col_to_rmi<O>(house, house_cell);
			case col: return row_col_to_rmi<O>(house_cell, house);
			case box: return box_cell_to_rmi<O>(house, house_cell);
		}
		OKIIDOKU_UNREACHABLE;
	}

	template<Order O, class T_chute, class T_chute_cell>
	requires(Any_o1x_t<O, T_chute> && Any_o3x_t<O, T_chute_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> chute_cell_to_rmi(const LineType line_type, const T_chute chute, const T_chute_cell chute_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_USE(chute < T::O1);
		OKIIDOKU_CONTRACT_USE(chute_cell < T::O3);
		switch (line_type) {
			using enum LineType;
			case row: return T::o4x(T::o4x(T::O3*chute)+chute_cell);
			case col: return T::o4x((T::O1*chute)+((chute_cell%T::O2)*T::O2)+(chute_cell/T::O2));
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

	namespace int_ts {
		using o1x_t = mono::int_ts::o1x_t<largest_compiled_order>;
		using o1i_t = mono::int_ts::o1i_t<largest_compiled_order>;

		using o2x_t = mono::int_ts::o2x_t<largest_compiled_order>;
		using o2i_t = mono::int_ts::o2i_t<largest_compiled_order>;
		using o2xs_t = mono::int_ts::o2xs_t<largest_compiled_order>;
		using o2is_t = mono::int_ts::o2is_t<largest_compiled_order>;

		using o4x_t = mono::int_ts::o4x_t<largest_compiled_order>;
		using o4i_t = mono::int_ts::o4i_t<largest_compiled_order>;
		using o4xs_t = mono::int_ts::o4xs_t<largest_compiled_order>;
		using o4is_t = mono::int_ts::o4is_t<largest_compiled_order>;
	}
	using grid_val_t = mono::grid_val_t<largest_compiled_order>;
}
#endif