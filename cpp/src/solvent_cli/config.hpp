#ifndef HPP_SOLVENT_CLI_CONFIG
#define HPP_SOLVENT_CLI_CONFIG

#include <solvent_cli/enum.hpp>
#include <solvent_lib/gen/batch.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/size.hpp>

#include <map>
#include <string>
#include <array>

namespace solvent::cli {

	class Config {
	 public:
		using pathkind_t = lib::gen::path::Kind;

		[[gnu::pure]] pathkind_t path_kind(void) const noexcept { return path_kind_; }
		void path_kind(pathkind_t) noexcept;
		void path_kind(std::string const&) noexcept;

		[[gnu::pure]] verbosity::Kind verbosity(void) const noexcept { return verbosity_; };
		void verbosity(verbosity::Kind);
		void verbosity(std::string const&);

		[[gnu::pure]] unsigned long long max_dead_ends(void) const noexcept { return max_dead_ends_; };
		void max_dead_ends(unsigned long long);
		void max_dead_ends(std::string const&);

	 private:
		verbosity::Kind verbosity_;
		pathkind_t path_kind_;
		unsigned long long max_dead_ends_;
	};
}
#endif