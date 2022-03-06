#ifndef HPP_SOLVENT_LIB__GEN__PATH
#define HPP_SOLVENT_LIB__GEN__PATH

#include "solvent_lib/size.hpp"
#include "solvent_config.hpp"

#include <iosfwd>
#include <array>
#include <string_view>

namespace solvent::lib::gen::path {

	enum class Kind : std::uint8_t {
		RowMajor,
		BlockCol,
		DealRwMj,
		__MAX__ = DealRwMj,
	};
	constexpr size_t num_kinds = static_cast<size_t>(Kind::__MAX__) + 1;
	// Indices of entries must match the
	// literal values of their respective enums.
	constexpr std::array<std::string_view, num_kinds> names {
		"rowmajor",
		"blockcol",
		"dealrwmj",
	};
	extern std::ostream& operator<<(std::ostream& os, Kind path_kind);
	constexpr std::string_view options_menu_str {"\nGEN-PATH OPTIONS:"
		"\n- rowmajor   horizontal strips as wide as the grid one by one"
		"\n- blockcol   rowmajor, but broken into columns one block wide"
		"\n- dealrwmj   like dealing cards to each block using row-major"};


	// input progress must be _less than_ O4.
	template<solvent::Order O>
	using coord_converter_t = [[gnu::const]] typename size<O>::ord4x_t (&)(typename size<O>::ord4x_t) noexcept;

	template<solvent::Order O> [[nodiscard, gnu::const]]
	coord_converter_t<O> get_prog_to_coord_converter(Kind) noexcept;

	template<solvent::Order O> [[nodiscard, gnu::const]]
	coord_converter_t<O> get_coord_to_prog_converter(Kind) noexcept;


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template coord_converter_t<O_> get_prog_to_coord_converter<O_>(Kind) noexcept; \
		extern template coord_converter_t<O_> get_coord_to_prog_converter<O_>(Kind) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif