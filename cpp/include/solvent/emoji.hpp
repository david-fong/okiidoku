#ifndef HPP_SOLVENT__EMOJI
#define HPP_SOLVENT__EMOJI

#include "solvent_export.h"

#include <vector>
#include <string_view>

namespace solvent::emoji {
	struct Set {
		std::string_view name;
		std::vector<std::string_view> entries;
	};
	SOLVENT_EXPORT extern const std::vector<Set> sets;
	SOLVENT_EXPORT extern const std::vector<size_t> top_set_preferences;
}
#endif