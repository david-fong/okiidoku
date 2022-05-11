#ifndef HPP_OKIIDOKU__DB__MOD
#define HPP_OKIIDOKU__DB__MOD

#include <okiidoku/detail/order_templates.hpp>

#include <iosfwd>
#include <string_view>
#include <optional>

namespace okiidoku::mono::db {

	class Db final {
	public:
		// disallow copies.
		Db(const Db&) = delete;
		Db& operator=(const Db&) = delete;
	};

	struct Header final {
		bool is_locked; // true if another operation is currently using this Db already.
		std::uint8_t order;
		bool is_solved;
		bool is_canonicalized;
	};

	// contract: 
	OKIIDOKU_EXPORT std::optional<Db> open_file(std::string_view filename);
}
#endif