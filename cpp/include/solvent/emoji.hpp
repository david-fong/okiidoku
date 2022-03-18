#ifndef HPP_SOLVENT__EMOJI
#define HPP_SOLVENT__EMOJI

#include "solvent_export.h"

#include <vector>
#include <string_view>

namespace solvent {
	struct EmojiSet {
		std::string_view name;
		std::vector<std::string_view> entries;
	};
	SOLVENT_EXPORT extern const std::vector<EmojiSet> emoji_sets;
}
#endif