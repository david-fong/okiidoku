#ifndef HPP_SOLVENT__GEN__PATH
#define HPP_SOLVENT__GEN__PATH

#include "solvent/size.hpp"
#include "solvent_config.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <array>
#include <string_view>

namespace solvent::lib::gen::path {

	enum class SOLVENT_EXPORT E : std::uint8_t {
		RowMajor,
		BlockCol,
		DealRwMj,
		max_ = DealRwMj,
	};
	constexpr size_t num_kinds = static_cast<size_t>(E::max_) + 1;
	// Indices of entries must match the
	// literal values of their respective enums.
	constexpr std::array<std::string_view, num_kinds> names {
		"rowmajor",
		"blockcol",
		"dealrwmj",
	};
	SOLVENT_EXPORT extern std::ostream& operator<<(std::ostream& os, E path_kind);
	constexpr std::string_view options_menu_str {"\nGEN-PATH OPTIONS:"
		"\n- rowmajor   horizontal strips as wide as the grid one by one"
		"\n- blockcol   rowmajor, but broken into columns one block wide"
		"\n- dealrwmj   like dealing cards to each block using row-major"};


	// input progress must be _less than_ O4.
	template<solvent::Order O>
	using coord_converter_t = [[gnu::const]] typename size<O>::ord4x_t (&)(typename size<O>::ord4x_t) noexcept;

	template<solvent::Order O> [[nodiscard, gnu::const]]
	SOLVENT_EXPORT coord_converter_t<O> get_prog_to_coord_converter(E) noexcept;

	template<solvent::Order O> [[nodiscard, gnu::const]]
	SOLVENT_EXPORT coord_converter_t<O> get_coord_to_prog_converter(E) noexcept;


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template coord_converter_t<O_> get_prog_to_coord_converter<O_>(E) noexcept; \
		extern template coord_converter_t<O_> get_coord_to_prog_converter<O_>(E) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif