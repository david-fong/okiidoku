#ifndef HPP_OKIIDOKU__INTS
#define HPP_OKIIDOKU__INTS
// Note: the byte width type logic could be done with boost, but I don't
// have any other reason to add boost as a dependency, so I won't.

#include <okiidoku/detail/order_templates.hpp> // Order, largest_compiled_order

#include <array>
#include <bit>         // bit_width
#include <cstdint>     // uint_...
#include <limits>      // numeric_limits<T>::max
#include <type_traits> // conditional_t
#include <concepts>    // unsigned_integral

namespace okiidoku {

	enum class HouseType : unsigned char {
		box, row, col,
	};

	constexpr auto house_types {std::to_array<HouseType>({
		HouseType::row,
		HouseType::col,
		HouseType::box
	})};
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


	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_row(const typename Ints<O>::o4i_t index) noexcept { return static_cast<typename Ints<O>::o2i_t>(index / (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_col(const typename Ints<O>::o4i_t index) noexcept { return static_cast<typename Ints<O>::o2i_t>(index % (Ints<O>::O2)); }
	template<Order O> [[nodiscard, gnu::const]] constexpr typename Ints<O>::o2i_t rmi_to_box(const typename Ints<O>::o2i_t row, const typename Ints<O>::o2i_t col) noexcept {
		return static_cast<typename Ints<O>::o2i_t>((row / O) * O) + (col / O);
	}
	template<Order O> [[nodiscard, gnu::const]]
	constexpr typename Ints<O>::o2i_t rmi_to_box(const typename Ints<O>::o4i_t index) noexcept {
		return rmi_to_box<O>(rmi_to_row<O>(index), rmi_to_col<O>(index));
	}
	template<Order O, class T_rmi>
	requires(Any_o4x<O, T_rmi>) [[nodiscard, gnu::const]]
	constexpr typename Ints<O>::o2x_t rmi_to_house(const HouseType house_type, const T_rmi rmi) noexcept {
		using T = Ints<O>;
		using o2x_t = typename T::o2x_t;
		switch (house_type) {
		case HouseType::row: return static_cast<o2x_t>(rmi_to_row<O>(rmi));
		case HouseType::col: return static_cast<o2x_t>(rmi_to_col<O>(rmi));
		case HouseType::box: return static_cast<o2x_t>(rmi_to_box<O>(rmi));
		}
	}


	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x<O, T_house> && Any_o2x<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr typename Ints<O>::o4i_t box_cell_to_rmi(const T_house box, const T_house_cell box_cell) noexcept {
		using T = Ints<O>;
		const auto row {static_cast<typename T::o4i_t>(((box/T::O1)*T::O1) + (box_cell/T::O1))};
		const auto col {static_cast<typename T::o4i_t>(((box%T::O1)*T::O1) + (box_cell%T::O1))};
		return (Ints<O>::O4 * row) + col;
	}

	template<Order O, class T_house, class T_house_cell>
	requires(Any_o2x<O, T_house> && Any_o2x<O, T_house_cell>) [[nodiscard, gnu::const]]
	constexpr typename Ints<O>::o4i_t house_cell_to_rmi(const HouseType house_type, const T_house house, const T_house_cell house_cell) noexcept {
		using T = Ints<O>;
		using o4i_t = typename T::o4i_t;
		switch (house_type) {
		case HouseType::row: return static_cast<o4i_t>((T::O2*house)+house_cell);
		case HouseType::col: return static_cast<o4i_t>((T::O2*house_cell)+house);
		case HouseType::box: return box_cell_to_rmi<O>(house, house_cell);
		}
	}
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