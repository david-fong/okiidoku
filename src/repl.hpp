#ifndef HPP_SUDOKU_REPL
#define HPP_SUDOKU_REPL

#include <map>

#include "solver.hpp"


namespace Sudoku {

    enum Command {
        CMD_HELP,
        CMD_QUIT,
        CMD_SOLVE,
        CMD_RUN_SINGLE,
        CMD_CONTINUE_PREV,
        CMD_RUN_TRIALS,
        CMD_RUN_SUCCESSES,
        CMD_SET_GENPATH,
    };

    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       CMD_HELP            },
        { "quit",       CMD_QUIT            },
        { "solve",      CMD_SOLVE           },
        { "",           CMD_RUN_SINGLE      },
        { "cont",       CMD_CONTINUE_PREV   },
        { "trials",     CMD_RUN_TRIALS      },
        { "strials",    CMD_RUN_SUCCESSES   },
        { "genpath",    CMD_SET_GENPATH     },
    };

    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help               print this help menu"
        "\n- quit               terminate this program"
        "\n- solve <file>       solve the puzzles in <file>"
        "\n- solve <puzzle>     no spaces; zeros mean empty"
        "\n- {enter}            generate a single solution"
        "\n- cont               continue previous generation"
        "\n- trials <n>         attempt to generate <n> solutions"
        "\n- strials <n>        successfully generate <n> solutions"
        "\n- genpath            cycle generator traversal path"
        ;
    const std::string REPL_PROMPT = "\n$ ";

    typedef unsigned long trials_t;
    constexpr unsigned int TRIALS_NUM_BINS = 20;
    enum TrialsStopBy {
        TOTAL_TRIALS,
        TOTAL_SUCCESSES, // TODO make a a command for this
    };

    volatile unsigned int GET_TERM_COLS(const unsigned fallback) noexcept {
        char const*const envVar = std::getenv("COLUMNS");
        return (envVar != NULL) ? std::stoul(envVar) : fallback;
    }


    /**
     * 
     * 
     * Notes for me as I learn how to write inheritance in C++:
     * I can specify base-class members like:
     * - Derived::Base::member
     *   https://en.cppreference.com/w/cpp/language/injected-class-name
     * - this.Base->member
     * - this->member
     * - Base<ARGS>::member
     */
    template <Order O, bool CBT, GiveupMethod GUM>
    class Repl {
    public:
        explicit Repl(std::ostream&);
        bool runCommand(std::string const& cmdLine);

    private:
        Solver<O,CBT,GUM> solver;
        std::ostream& os;

        // Return false if command is to exit the program:
        void solvePuzzlesFromFile(std::ifstream&);
        void runSingle(bool contPrev = false);
        void runMultiple(unsigned long numTrials, TrialsStopBy);

        void printTrialsWorkDistribution(const trials_t numTotalTrials,
            std::array<trials_t, TRIALS_NUM_BINS+1> const& binHitCount,
            std::array<double,   TRIALS_NUM_BINS+1> const& binOpsTotal);
    };

}

#endif