#ifndef HPP_OKIIDOKU__MONO__DB__MOD
#define HPP_OKIIDOKU__MONO__DB__MOD

#include <okiidoku/order.hpp>
#include <okiidoku_export.h>

#include <iosfwd>
#include <string_view>
#include <span>
#include <bit>

namespace okiidoku::mono::db {

	class Db final {
		;
	};
	struct Header final {
		bool is_locked; // true if another operation is currently using this Db already.
		std::uint8_t order;
		bool is_solved;
		bool is_canonicalized;
	};

	OKIIDOKU_EXPORT bool open_or_create(std::string_view);
}
#endif