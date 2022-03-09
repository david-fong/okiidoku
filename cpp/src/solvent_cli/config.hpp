#ifndef HPP_SOLVENT_CLI__CONFIG
#define HPP_SOLVENT_CLI__CONFIG

#include "solvent_cli/enum.hpp"
// #include "solvent/gen/batch.hpp"
#include "solvent/gen/backtracking.hpp"
#include "solvent/size.hpp"

#include <map>
#include <string_view>
#include <array>

namespace solvent::cli {

	class Config {
	public:
		using pathkind_t = lib::gen::path::E;

		[[nodiscard, gnu::pure]] verbosity::E verbosity(void) const noexcept { return verbosity_; };
		void verbosity(verbosity::E);
		void verbosity(std::string_view);

		[[nodiscard, gnu::pure]] Order order(void) const noexcept { return order_; }
		void order(Order) noexcept;
		void order(std::string_view);

		[[nodiscard, gnu::pure]] pathkind_t path_kind(void) const noexcept { return path_kind_; }
		void path_kind(pathkind_t) noexcept;
		void path_kind(std::string_view) noexcept;

		[[nodiscard, gnu::pure]] unsigned long long max_dead_ends(void) const noexcept { return max_dead_ends_; };
		void max_dead_ends(unsigned long long);
		void max_dead_ends(std::string_view);

		[[nodiscard, gnu::pure]] bool canonicalize(void) const noexcept { return canonicalize_; };
		void canonicalize(bool);
		void canonicalize(std::string_view);

	private:
		Order order_;
		verbosity::E verbosity_ {verbosity::E::quiet_aborts};
		pathkind_t path_kind_ {pathkind_t::row_major};
		unsigned long long max_dead_ends_ {0};
		bool canonicalize_ {false};
	};
}
#endif