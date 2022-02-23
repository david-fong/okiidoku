#ifndef HPP_SOLVENT_LIB__GEN__PATH
#define HPP_SOLVENT_LIB__GEN__PATH

#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

#include <iosfwd>
#include <array>
#include <string>

namespace solvent::lib::gen::path {

	enum class Kind : std::uint8_t {
		RowMajor,
		BlockCol,
		DealRwMj,
		__MAX__ = DealRwMj,
	};
	constexpr size_t NUM_KINDS = static_cast<size_t>(Kind::__MAX__) + 1;
	// Indices of entries must match the
	// literal values of their respective enums.
	inline const std::array<std::string, NUM_KINDS> NAMES = {
		"rowmajor",
		"blockcol",
		"dealrwmj",
	};
	extern std::ostream& operator<<(std::ostream& os, Kind path_kind);
	inline const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
		"\n- rowmajor   horizontal strips as wide as the grid one by one"
		"\n- blockcol   rowmajor, but broken into columns one block wide"
		"\n- dealrwmj   like dealing cards to each block using row-major";


	// input progress must be _less than_ O4.
	template<solvent::Order O>
	using coord_converter_t = [[gnu::const]] typename size<O>::ord4_t (&)(typename size<O>::ord4_t) noexcept;

	template<solvent::Order O> [[nodiscard]]
	coord_converter_t<O> GetPathCoords(Kind) noexcept;


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template coord_converter_t<O_> GetPathCoords<O_>(Kind) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif