#ifndef HPP_OKIIDOKU__INTS
#define HPP_OKIIDOKU__INTS

#include <okiidoku/detail/order_templates.hpp> // Order, largest_compiled_order
#include <okiidoku/detail/contract.hpp>

#include <array>
#include <bit>         // bit_width
#include <cstdint>     // uint_...
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral

namespace okiidoku {

	using rng_seed_t = std::uint_least64_t;

	enum class HouseType : unsigned char {
		box, row, col,
	};
	constexpr auto house_types {std::to_array<HouseType>({
		HouseType::row,
		HouseType::col,
		HouseType::box
	})};

	enum class LineType : unsigned char {
		row, col,
	};
	constexpr auto line_types {std::to_array<LineType>({
		LineType::row,
		LineType::col
	})};

	enum class BoxOrLine : unsigned char {
		box, line,
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

	namespace int_ts {

		template<Order O> using o1x_t = detail::uint_fastN_t<std::bit_width(O)>;
		template<Order O> using o1i_t = detail::uint_fastN_t<std::bit_width(O)>;

		template<Order O> using o2x_t = detail::uint_fastN_t<std::bit_width(O*O-1)>;
		template<Order O> using o2i_t = detail::uint_fastN_t<std::bit_width(O*O)>;
		template<Order O> using o2xs_t = detail::uint_smolN_t<std::bit_width(O*O-1)>;
		template<Order O> using o2is_t = detail::uint_smolN_t<std::bit_width(O*O)>;

		template<Order O> using o3x_t = detail::uint_fastN_t<std::bit_width(O*O*O-1)>;
		template<Order O> using o3i_t = detail::uint_fastN_t<std::bit_width(O*O*O)>;
		template<Order O> using o3xs_t = detail::uint_smolN_t<std::bit_width(O*O*O-1)>;
		template<Order O> using o3is_t = detail::uint_smolN_t<std::bit_width(O*O*O)>;

		template<Order O> using o4x_t = detail::uint_fastN_t<std::bit_width(O*O*O*O-1)>;
		template<Order O> using o4i_t = detail::uint_fastN_t<std::bit_width(O*O*O*O)>;
		template<Order O> using o4xs_t = detail::uint_smolN_t<std::bit_width(O*O*O*O-1)>;
		template<Order O> using o4is_t = detail::uint_smolN_t<std::bit_width(O*O*O*O)>;

		template<Order O> using o5i = detail::uint_fastN_t<std::bit_width(O*O*O*O*O)>;

		template<Order O> using o6i = detail::uint_fastN_t<std::bit_width(O*O*O*O*O*O)>;
	}

	// Note: when printing things, make sure to cast to int, since byte-like types will be interpreted as characters.
	template<Order O>
	struct Ints final {

		Ints() = delete;

		static constexpr int_ts::o1i_t<O> O1 {O};
		static constexpr int_ts::o2i_t<O> O2 {O*O};
		static constexpr int_ts::o3i_t<O> O3 {O*O*O};
		static constexpr int_ts::o4i_t<O> O4 {O*O*O*O};
	};

	// TODO.low consider changing all these allow signed integers as well? Not sure what pros and cons are.
	template<Order O, typename T>
	concept Any_o1x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O1-1);

	template<Order O, typename T>
	concept Any_o2x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O2-1);

	template<Order O, typename T>
	concept Any_o3x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O3-1);

	template<Order O, typename T>
	concept Any_o4x_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() >= (Ints<O>::O4-1);


	template<Order O>
	using grid_val_t = int_ts::o2is_t<O>;


	template<Order O> [[nodiscard, gnu::const]] constexpr int_ts::o2i_t<O> rmi_to_row(const int_ts::o4i_t<O> rmi) noexcept { return static_cast<int_ts::o2i_t<O>>(rmi / (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr int_ts::o2i_t<O> rmi_to_col(const int_ts::o4i_t<O> rmi) noexcept { return static_cast<int_ts::o2i_t<O>>(rmi % (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr int_ts::o2i_t<O> rmi_to_box(const int_ts::o2i_t<O> row, const int_ts::o2i_t<O> col) noexcept {
		return static_cast<int_ts::o2i_t<O>>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2i_t<O> rmi_to_box(const int_ts::o4i_t<O> rmi) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(rmi), rmi_to_col<O>(rmi));
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr int_ts::o2i_t<O> rmi_to_box_cell(const int_ts::o4i_t<O> rmi) noexcept {
		using T = Ints<O>;
		const auto boxrow {(rmi/T::O2)%T::O1};
		const auto boxcol {rmi%T::O1};
		return static_cast<int_ts::o2i_t<O>>((T::O1*boxrow)+boxcol);
	}
	template<Order O, class T_rmi>
	requires(Any_o4x_t<O, T_rmi>) [[nodiscard, gnu::const]]
	constexpr int_ts::o2x_t<O> rmi_to_house(const HouseType house_type, const T_rmi rmi) noexcept {
		using o2x_t = int_ts::o2x_t<O>;
		switch (house_type) {
		case HouseType::row: return static_cast<o2x_t>(rmi_to_row<O>(rmi));
		case HouseType::col: return static_cast<o2x_t>(rmi_to_col<O>(rmi));
		case HouseType::box: return static_cast<o2x_t>(rmi_to_box<O>(rmi));
		}
	}


	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> box_cell_to_rmi(const T_house box, const T_house_cell box_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(box < T::O2);
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(box_cell < T::O2);
		using o4x_t = int_ts::o4x_t<O>;
		const auto row {static_cast<o4x_t>(((box/T::O1)*T::O1) + (box_cell/T::O1))};
		const auto col {static_cast<o4x_t>(((box%T::O1)*T::O1) + (box_cell%T::O1))};
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(row < T::O2);
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(col < T::O2);
		const auto rmi {static_cast<o4x_t>((Ints<O>::O2 * row) + col)};
		return rmi;
	}

	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x_t<O, T_house> && Any_o2x_t<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> house_cell_to_rmi(const HouseType house_type, const T_house house, const T_house_cell house_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(house < T::O2);
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(house_cell < T::O2);
		using o4x_t = int_ts::o4x_t<O>;
		switch (house_type) {
		case HouseType::row: return static_cast<o4x_t>((T::O2*house)+house_cell);
		case HouseType::col: return static_cast<o4x_t>((T::O2*house_cell)+house);
		case HouseType::box: return box_cell_to_rmi<O>(house, house_cell);
		}
	}

	template<Order O, class T_chute, class T_chute_cell>
	requires(Any_o1x_t<O, T_chute> && Any_o3x_t<O, T_chute_cell>) [[nodiscard, gnu::const]]
	constexpr int_ts::o4x_t<O> chute_cell_to_rmi(const LineType line_type, const T_chute chute, const T_chute_cell chute_cell) noexcept {
		using T = Ints<O>;
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(chute < T::O1);
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(chute_cell < T::O3);
		using o4x_t = int_ts::o4x_t<O>;
		switch (line_type) {
		case LineType::row: return static_cast<o4x_t>((T::O3*chute)+chute_cell);
		case LineType::col: return static_cast<o4x_t>((T::O1*chute)+((chute_cell%T::O2)*T::O2)+(chute_cell/T::O2));
		}
	}
}


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

		using o5i = mono::int_ts::o5i<largest_compiled_order>;

		using o6i = mono::int_ts::o6i<largest_compiled_order>;
	}
	using grid_val_t = mono::grid_val_t<largest_compiled_order>;
}
#endif