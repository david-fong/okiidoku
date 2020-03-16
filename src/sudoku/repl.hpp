#ifndef HPP_SUDOKU_REPL
#define HPP_SUDOKU_REPL

#include "./solver.hpp"

#include <map>


namespace Sudoku {

    enum class Command {
        HELP,
        QUIT,
        RUN_SINGLE,
        CONTINUE_PREV,
        RUN_TRIALS,
        RUN_SUCCESSES,
        SET_GENPATH,
        SOLVE,
    };

    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       Command::HELP           },
        { "quit",       Command::QUIT           },
        { "",           Command::RUN_SINGLE     },
        { "cont",       Command::CONTINUE_PREV  }, // TODO Change this to "c"? (remember to change help string too)
        { "trials",     Command::RUN_TRIALS     },
        { "strials",    Command::RUN_SUCCESSES  },
        { "genpath",    Command::SET_GENPATH    },
        { "solve",      Command::SOLVE          },
    };
    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help               print this help menu"
        "\n- quit               terminate this program"
        "\n- {enter}            generate a single solution"
        "\n- cont               continue previous generation"
        "\n- trials <n>         attempt to generate <n> solutions"
        "\n- strials <n>        successfully generate <n> solutions"
        "\n- genpath [<path>]   get/set generator traversal path"
        "\n- solve <file>       solve the puzzles in <file>"
        "\n- solve <puzzle>     no spaces; zeros mean empty"
        ;

    const std::string REPL_PROMPT = "\n$ ";

    typedef unsigned long trials_t;
    constexpr unsigned int TRIALS_NUM_BINS = 20;
    enum class TrialsStopBy {
        TRIALS,
        SUCCESSES,
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
    template <Order O, bool CBT, GUM::E GUM>
    class Repl {
    public:
        explicit Repl(std::ostream&);
        static void SEED(unsigned);
        bool runCommand(std::string const& cmdLine);
        typedef class Solver<O,CBT,GUM> solver_t;

        static constexpr unsigned MAX_EXTRA_THREADS = ((const unsigned[]){0, 0, 0, 0, 1, 2, 3})[O];
        // This is equal to `MAX_EXTRA_THREADS` floored by how many
        // concurrent threads the host processor can support at a time.
        const unsigned numExtraThreads;

    private:
        solver_t solver;
        std::ostream& os; // alias to this->solver.os;

        // Return false if command is to exit the program:
        void solvePuzzlesFromFile(std::ifstream&);
        void runSingle(bool contPrev = false);
        void runMultiple(unsigned long numTrials, TrialsStopBy);
        void runMultiple(std::string const&, TrialsStopBy);

        void printTrialsWorkDistribution(const trials_t numTotalTrials,
            std::array<trials_t, TRIALS_NUM_BINS+1> const& binHitCount,
            std::array<double,   TRIALS_NUM_BINS+1> const& binOpsTotal);

    }; // End of Repl class

} // End of Sudoku namespace

#endif