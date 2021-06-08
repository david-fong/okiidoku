#ifndef HPP_SOLVENT_LIB_GEN_PATH
#define HPP_SOLVENT_LIB_GEN_PATH

#include <solvent_lib/size.hpp>

#include <iosfwd>
// #include <numeric> // iota
#include <array>
#include <string>

namespace solvent::lib::gen {

	namespace path {
		enum class Kind : unsigned {
			RowMajor,
			DealRwMj,
			BlockCol,
			__MAX__ = BlockCol,
		};
		constexpr size_t NUM_KINDS = static_cast<size_t>(Kind::__MAX__) + 1;
		// Indices of entries must match the
		// literal values_ of their respective enums.
		inline const std::array<std::string, NUM_KINDS> NAMES = {
			"rowmajor",
			"dealrwmj",
			"blockcol",
		};
		extern std::ostream& operator<<(std::ostream& os, Kind path_kind);
		inline const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
			"\n- rowmajor   horizontal strips as wide as the grid one by one"
			"\n- dealrwmj   like dealing cards to each block using row-major"
			"\n- blockcol   rowmajor, but broken into columns one block wide";

		template<solvent::Order O>
		using coord_converter_t = typename size<O>::ord4_t (&)(typename size<O>::ord4_t);

		template<solvent::Order O>
		coord_converter_t<O> GetPathCoords(Kind) noexcept;


		#define SOLVENT_TEMPL_TEMPL(O_) \
			extern template coord_converter_t<O_> GetPathCoords<O_>(Kind) noexcept;
		SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef SOLVENT_TEMPL_TEMPL
	}
}
#endif