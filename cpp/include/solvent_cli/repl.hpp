#ifndef HPP_SOLVENT_CLI__REPL
#define HPP_SOLVENT_CLI__REPL

#include <solvent_cli/config.hpp>
#include <solvent_cli/enum.hpp>
#include <solvent_lib/toolkit.hpp>
#include <solvent_lib/gen/batch.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/size.hpp>

#include <map>
#include <string>
#include <array>


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
		const std::map<std::string, Command::E> Str2Enum = {
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
		const std::string HelpMessage = "\nCOMMAND MENU:"
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
			;
	}

	// Returns zero on error.
	inline unsigned GET_TERM_COLS(void) noexcept {
		char const*const env_var = std::getenv("COLUMNS");
		return (env_var != NULL) ? std::stoul(env_var) : 0u;
	}


	/**
	 * Notes for me as I learn how to write inheritance in C++:
	 * I can specify base-class members like:
	 * - Derived::Base::member
	 *   https://en.cppreference.com/w/cpp/language/injected-class-name
	 * - this.Base->member
	 * - this->member
	 * - Base<ARGS>::member
	 */
	class Repl final {
	 public:
		using opcount_t = lib::gen::opcount_t;
		using pathkind_t = lib::gen::path::Kind;
		using trials_t = lib::gen::batch::trials_t;

		Repl(Order O);
		void start(void);
		bool run_command(const std::string& cmd_line);

	 private:
		Config config_;
		lib::Toolkit toolkit_;

		// Return false if command is to exit the program:
		void gen_single(bool contPrev = false);

		void gen_multiple(trials_t stop_after, bool only_count_oks);
		void gen_multiple(const std::string&,  bool only_count_oks);
	};
}
#endif