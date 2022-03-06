#ifndef HPP_SOLVENT_CLI__REPL
#define HPP_SOLVENT_CLI__REPL

#include "solvent_cli/config.hpp"
#include "solvent_cli/enum.hpp"
#include "solvent_lib/gen/batch.hpp"
#include "solvent_lib/gen/mod.hpp"
#include "solvent_lib/size.hpp"

#include <map>
#include <string_view>
#include <array>
#include <memory> // unique_ptr


namespace solvent::cli {

	namespace Command {
		enum class E : unsigned {
			Help,
			Quit,
			ConfigOrder,
			ConfigVerbosity,
			ConfigGenPath,
			ConfigMaxDeadEnds,
			Canonicalize,
			GenSingle,
			GenContinue,
			GenMultiple,
			GenMultipleOk,
		};
		const std::map<std::string_view, Command::E> Str2Enum {
			{ "help",         E::Help              },
			{ "quit",         E::Quit              },
			{ "verbosity",    E::ConfigVerbosity   },
			{ "order",        E::ConfigOrder       },
			{ "genpath",      E::ConfigGenPath     },
			{ "maxdeadends",  E::ConfigMaxDeadEnds },
			{ "canonicalize", E::Canonicalize      },
			{ "",             E::GenSingle         },
			{ "c",            E::GenContinue       },
			{ "gen",          E::GenMultiple       },
			{ "gen_ok",       E::GenMultipleOk     },
		};
		constexpr std::string_view HelpMessage {"\nCOMMAND MENU:"
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
		using opcount_t = lib::gen::opcount_t;
		using pathkind_t = lib::gen::path::Kind;
		using trials_t = lib::gen::batch::trials_t;

		explicit Repl(Order O);
		void start(void);
		bool run_command(std::string_view cmd_line);

	 private:
		Config config_;
		std::unique_ptr<lib::gen::Generator> gen_;

		// Return false if command is to exit the program:
		void gen_single(bool contPrev = false);

		void gen_multiple(trials_t stop_after, bool only_count_oks);
		void gen_multiple(std::string_view,  bool only_count_oks);
	};
}
#endif