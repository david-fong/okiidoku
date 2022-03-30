#ifndef HPP_OOKIIDOKU__EMOJI
#define HPP_OOKIIDOKU__EMOJI

#include <ookiidoku_export.h>

#include <vector>
#include <string_view>

namespace ookiidoku::emoji {
	struct Set {
		std::string_view name;
		std::vector<std::string_view> entries;
	};
	OOKIIDOKU_EXPORT extern const std::vector<Set> sets;
	OOKIIDOKU_EXPORT extern const std::vector<size_t> top_set_preferences;
}
#endif