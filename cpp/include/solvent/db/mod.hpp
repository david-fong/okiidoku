#ifndef HPP_SOLVENT__DB__MOD
#define HPP_SOLVENT__DB__MOD

#include "solvent/order.hpp"
#include "solvent_export.h"

#include <iosfwd>
#include <string_view>
#include <span>
#include <bit>

namespace solvent::db {

	class Db {
		;
	};
	struct Header {
		bool is_locked; // true if another operation is currently using this Db already.
		std::uint8_t order;
		bool is_solved;
		bool is_canonicalized;
	};

	SOLVENT_EXPORT bool open_or_create(std::string_view);
}
#endif