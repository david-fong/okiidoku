#ifndef HPP_SOLVENT_CLI__REPL
#define HPP_SOLVENT_CLI__REPL

#include "solvent_cli/config.hpp"
#include "solvent_cli/enum.hpp"
#include "solvent/gen/bt/batch.hpp"
#include "solvent/gen/bt/generator.hpp"
#include "solvent/size.hpp"

#include <map>
#include <string_view>
#include <array>
#include <memory> // unique_ptr


namespace solvent::cli {

	namespace Command {
		enum class E {
			help,
			quit,
			config_order,
			config_print_level,
			config_gen_path,
			config_gen_max_dead_ends,
			config_auto_canonicalize,
			gen_single,
			gen_continue,
			gen_multiple,
			gen_multiple_ok,
		};
		const std::map<std::string_view, Command::E> enum_str_to_enum {
			{ "help",         E::help },
			{ "quit",         E::quit },
			{ "order",        E::config_order },
			{ "verbosity",    E::config_print_level },
			{ "genpath",      E::config_gen_path },
			{ "maxdeadends",  E::config_gen_max_dead_ends },
			{ "canonicalize", E::config_auto_canonicalize },
			{ "",             E::gen_single },
			{ "c",            E::gen_continue },
			{ "gen",          E::gen_multiple },
			{ "gen_ok",       E::gen_multiple_ok },
		};
		constexpr std::string_view helpMessage {"\nCOMMAND MENU:"
		"\n- help                  print this help menu"
		"\n- quit                  cleanly exit this program"
		"\n"
		"\n- verbosity [<level>]   get/set verbosity level"
		"\n- order [<order>]       get/set order (sqrt of grid length)"
		"\n- genpath [<path>]      get/set generator traversal path"
		"\n- maxdeadends [<max>]   get/set generator max dead ends"
		"\n- canonicalize [<y/n>]  get/set canonicalization"
		"\n"
		"\n- {enter}               generate a single solution"
		"\n- c                     continue previous generation"
		"\n- gen <n>               attempt to generate <n> solutions"
		"\n- gen_ok <n>            successfully generate <n> solutions"
		};
	}

	// Returns zero on error.
	inline unsigned get_terminal_num_cols(void) noexcept {
		char const*const env_var = std::getenv("COLUMNS");
		return (env_var != NULL) ? static_cast<unsigned>(std::stoul(env_var)) : 0u;
	}


	/** */
	class Repl final {
	public:
		explicit Repl(Order O);
		void start(void);
		bool run_command(std::string_view cmd_line);

	private:
		Config config_;
		std::unique_ptr<gen::bt::Generator> gen_;

		// Return false if command is to exit the program:
		void gen_single(bool contPrev = false);

		void gen_multiple(gen::bt::batch::trials_t stop_after, bool only_count_oks);
		void gen_multiple(std::string_view, bool only_count_oks);
	};
}
#endif