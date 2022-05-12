#ifndef HPP_OKIIDOKU__DB__MOD
#define HPP_OKIIDOKU__DB__MOD

#include <okiidoku/detail/order_templates.hpp>

#include <iosfwd>
#include <string_view>
#include <optional>

// TODO.asap consider renaming. DB generally implies mutability,
// but I thought we weren't implementing that and instead having
// only merge operations. Maybe "archive" might be a better word.
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

	// post-condition: returns `std::nullopt` if the specified file doesn't
	// exist or is not a regular file. Otherwise returns a handle to an open DB.
	[[nodiscard]] OKIIDOKU_EXPORT std::optional<Db> open_file(std::string_view filename);
}
#endif