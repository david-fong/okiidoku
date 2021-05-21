#ifndef HPP_SOLVENT_CLI_REPL
#define HPP_SOLVENT_CLI_REPL

#include "./enum.hpp"
#include "../lib/gen/mod.hpp"
#include "../lib/canon/mod.hpp"
#include "./trials.hpp"

#include <array>
#include <string>
#include <map>


namespace solvent::cli {

	const std::string PROMPT = "\n$ ";

	namespace Command {
		enum class E : unsigned {
			Help,
			Quit,
			OutputLevel,
			SetGenPath,
			RunSingle,
			ContinuePrev,
			RunMultiple,
			RunMultipleOk,
		};
		const std::map<std::string, Command::E> MAP = {
			{ "help",       E::Help           },
			{ "quit",       E::Quit           },
			{ "output",     E::OutputLevel    },
			{ "genpath",    E::SetGenPath     },
			{ "",           E::RunSingle      },
			{ "cont",       E::ContinuePrev   }, // TODO [qol] Change this to "c"? (remember to change help string too)
			{ "trials",     E::RunMultiple    },
			{ "strials",    E::RunMultipleOk  },
		};
		const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
			"\n- help               print this help menu"
			"\n- quit               cleanly exit this program"
			"\n- output [<level>]   get / set output level"
			"\n- genpath [<path>]   get / set generator traversal path"
			"\n- {enter}            generate a single solution"
			"\n- cont               continue previous generation"
			"\n- trials <n>         attempt to generate <n> solutions"
			"\n- strials <n>        successfully generate <n> solutions";
	} // End of Command namespace

	// Returns zero on error.
	unsigned GET_TERM_COLS(void) noexcept {
		char const*const envVar = std::getenv("COLUMNS");
		return (envVar != NULL) ? std::stoul(envVar) : 0u;
	}


	/**
	 *
	 * Notes for me as I learn how to write inheritance in C++:
	 * I can specify base-class members like:
	 * - Derived::Base::member
	 *   https://en.cppreference.com/w/cpp/language/injected-class-name
	 * - this.Base->member
	 * - this->member
	 * - Base<ARGS>::member
	 */
	template <Order O>
	class Repl final {
	  public:
		using opcount_t = Solver::opcount_t;
		using generator_t = class solvent::lib::gen::Generator<O>;

		Repl(void) = delete;
		explicit Repl(std::ostream&);
		bool runCommand(std::string const& cmdLine);

		// This is equal to `MAX_EXTRA_THREADS` floored by how many
		// concurrent threads the host processor can support at a time.
		const unsigned numExtraThreads;

	  private:
		generator_t gen;
		std::ostream& os; // alias to this->gen.os;
		OutputLvl::E outputLvl;

		static constexpr unsigned MAX_EXTRA_THREADS = ((const unsigned[]){0,0,0,0,1,4,2,2})[O];
		const std::string DIM_ON  = (gen.isPretty ? Ansi::DIM.ON  : "");
		const std::string DIM_OFF = (gen.isPretty ? Ansi::DIM.OFF : "");

		// Return false if command is to exit the program:
		void runSingle(bool contPrev = false);

		void runMultiple(trials_t numTrials, Trials::StopBy);
		void runMultiple(std::string const&, Trials::StopBy);
		void printTrialsWorkDistribution(trials_t numTotalTrials,
			std::array<trials_t, Trials::NUM_BINS+1> const& binHitCount,
			std::array<double,   Trials::NUM_BINS+1> const& binOpsTotal);

	  public:
		[[gnu::cold]] OutputLvl::E getOutputLvl(void) const noexcept { return outputLvl; };
		[[gnu::cold]] OutputLvl::E setOutputLvl(OutputLvl::E);
		[[gnu::cold]] OutputLvl::E setOutputLvl(std::string const&);

	};

}

#endif