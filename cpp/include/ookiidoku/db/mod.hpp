#ifndef HPP_OOKIIDOKU__DB__MOD
#define HPP_OOKIIDOKU__DB__MOD

#include <ookiidoku/order.hpp>
#include <ookiidoku_export.h>

#include <iosfwd>
#include <string_view>
#include <span>
#include <bit>

namespace ookiidoku::db {

	class Db final {
		;
	};
	struct Header final {
		bool is_locked; // true if another operation is currently using this Db already.
		std::uint8_t order;
		bool is_solved;
		bool is_canonicalized;
	};

	OOKIIDOKU_EXPORT bool open_or_create(std::string_view);
}
#endif