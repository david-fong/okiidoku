#ifndef HPP_OKIIDOKU__FMT__EMOJI
#define HPP_OKIIDOKU__FMT__EMOJI

#include <okiidoku/detail/export.h>

#include <vector>
#include <string_view>

namespace okiidoku::emoji {
	struct Set final {
		std::string_view name;
		std::vector<std::string_view> entries;
	};
	/* OKIIDOKU_EXPORT */ extern const std::vector<Set> sets;
	/* OKIIDOKU_EXPORT */ extern const std::vector<size_t> top_set_preferences;
}
#endif