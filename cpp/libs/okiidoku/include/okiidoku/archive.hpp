// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__ARCHIVE
#define HPP_OKIIDOKU__ARCHIVE

#include <okiidoku/detail/order_templates.hpp>

// #include <iosfwd>
#include <string_view>
#include <optional>
#include <memory> // unique_ptr
#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

namespace okiidoku::mono::archive {

	class OKIIDOKU_EXPORT Reader final {
	public:
		Reader() noexcept = default; // TODO
		~Reader() noexcept = default;
		// disallow copies.
		Reader(const Reader&) = delete;
		Reader& operator=(const Reader&) = delete;

		Reader(Reader&&) = default;
		Reader& operator=(Reader&&) = default;

	private:
		class Impl;
		#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<Impl>> impl_;
		#else // fallback for MSVC
		std::unique_ptr<Impl> impl_;
		#endif
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