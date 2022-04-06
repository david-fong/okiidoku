#ifndef HPP_OKIIDOKU__GEN__PATH
#define HPP_OKIIDOKU__GEN__PATH

#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <iosfwd>
#include <array>
#include <string_view>

namespace okiidoku::gen::bt::path {

	enum class OKIIDOKU_EXPORT E : std::uint8_t {
		row_major,
		block_col,
		dealer_row_major,
		max_ = dealer_row_major,
	};
	constexpr size_t num_kinds = static_cast<size_t>(E::max_) + 1;
	// Indices of entries must match the
	// literal values of their respective enums.
	constexpr std::array<std::string_view, num_kinds> names {
		"row_major",
		"block_col",
		"dealer_row_major",
	};
	OKIIDOKU_EXPORT std::ostream& operator<<(std::ostream& os, E path_kind);
	constexpr std::string_view options_menu_str {"\nGEN-PATH OPTIONS:"
		"\n- row_major          horizontal strips as wide as the grid one by one"
		"\n- block_col          rowmajor, but broken into columns one block wide"
		"\n- dealer_row_major   like dealing cards to each block using row-major"};


	template<okiidoku::Order O>
	using coord_converter = [[gnu::const]] typename traits<O>::o4x_t (&)(typename traits<O>::o4x_t) noexcept;

	template<okiidoku::Order O> [[nodiscard, gnu::const]]
	OKIIDOKU_EXPORT coord_converter<O> get_prog_to_coord_converter(E) noexcept;

	template<okiidoku::Order O> [[nodiscard, gnu::const]]
	OKIIDOKU_EXPORT coord_converter<O> get_coord_to_prog_converter(E) noexcept;
}
#endif