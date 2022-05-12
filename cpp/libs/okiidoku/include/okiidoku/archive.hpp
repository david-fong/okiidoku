#ifndef HPP_OKIIDOKU__ARCHIVE
#define HPP_OKIIDOKU__ARCHIVE

#include <okiidoku/detail/order_templates.hpp>

// #include <iosfwd>
#include <string_view>
#include <optional>
#include <memory> // unique_ptr
#include <experimental/propagate_const>

namespace okiidoku::mono::archive {

	class Reader final {
		/* OKIIDOKU_NO_EXPORT */ class Impl;
		std::experimental::propagate_const<std::unique_ptr<Impl>> impl_;

	public:
		// disallow copies.
		Reader(const Reader&) = delete;
		Reader& operator=(const Reader&) = delete;
	};

	struct Header final {
		bool is_locked; // true if another operation is currently using this Reader already.
		std::uint8_t order;
		bool is_solved;
		bool is_canonicalized;
	};

	// post-condition: returns `std::nullopt` if the specified file doesn't
	// exist or is not a regular file. Otherwise returns a handle to an Reader.
	[[nodiscard]] OKIIDOKU_EXPORT std::optional<Reader> open_file(std::string_view filename);

	void create_merged(); // TODO.mid design signature
}
#endif