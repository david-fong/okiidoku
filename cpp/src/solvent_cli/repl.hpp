#ifndef HPP_SOLVENT_CLI_REPL
#define HPP_SOLVENT_CLI_REPL

#include <solvent_cli/enum.hpp>
#include <solvent_lib/gen/batch.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/size.hpp>

#include <map>
#include <string>
#include <array>


namespace solvent::cli {

	inline const std::string PROMPT = "\n$ ";

	namespace Command {
		enum class E : unsigned {
			Help,
			Quit,
			ConfigVerbosity,
			ConfigGenPath,
			GenSingle,
			GenContinue,
			GenMultiple,
			GenMultipleOk,
		};
		const std::map<std::string, Command::E> Str2Enum = {
			{ "help",       E::Help            },
			{ "quit",       E::Quit            },
			{ "output",     E::ConfigVerbosity },
			{ "genpath",    E::ConfigGenPath   },
			{ "",           E::GenSingle       },
			{ "cont",       E::GenContinue     }, // TODO [qol] Change this to "c"? (remember to change help string too)
			{ "gen",        E::GenMultiple     },
			{ "gen_ok",     E::GenMultipleOk   },
		};
		const std::string HelpMessage = "\nCOMMAND MENU:"
			"\n- help               print this help menu"
			"\n- quit               cleanly exit this program"
			"\n- output [<level>]   get / set output level"
			"\n- genpath [<path>]   get / set generator traversal path"
			"\n- {enter}            generate a single solution"
			"\n- cont               continue previous generation"
			"\n- gen <n>            attempt to generate <n> solutions"
			"\n- gen_ok <n>         successfully generate <n> solutions";
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
	template<Order O>
	class Repl final {
	 public:
		using opcount_t = lib::gen::opcount_t;
		using generator_t = typename lib::gen::Generator<O>;
		using pathkind_t = lib::gen::path::Kind;
		using trials_t = lib::gen::batch::trials_t;

		Repl(void);
		void start(void);
		bool run_command(std::string const& cmd_line);

		[[gnu::pure]] pathkind_t get_path_kind(void) const noexcept { return path_kind_; }
		// Setters return the old value of the generator path.
		pathkind_t set_path_kind(pathkind_t) noexcept;
		pathkind_t set_path_kind(std::string const&) noexcept;

		[[gnu::pure]] verbosity::Kind get_verbosity(void) const noexcept { return verbosity_; };
		// Setters return the old value of the verbosity.
		verbosity::Kind set_verbosity(verbosity::Kind);
		verbosity::Kind set_verbosity(std::string const&);

	 private:
		verbosity::Kind verbosity_;
		pathkind_t path_kind_;

		// Return false if command is to exit the program:
		void gen_single(bool contPrev = false);

		void gen_multiple(trials_t stop_after, bool only_count_oks);
		void gen_multiple(std::string const&,  bool only_count_oks);

		void print_msg_bar(std::string const&, unsigned int, std::string = "═") const;
		void print_msg_bar(std::string const&, std::string = "═") const;
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Repl<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif