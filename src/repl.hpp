#ifndef HPP_SUDOKU_REPL
#define HPP_SUDOKU_REPL

#include <map>

#include "solver.hpp"


namespace Sudoku {

    typedef enum {
        CMD_HELP,
        CMD_QUIT,
        CMD_SOLVE,
        CMD_RUN_SINGLE,
        CMD_RUN_MULTIPLE,
        CMD_SET_GENPATH,
        CMD_DO_BACKTRACK_COUNT,
    } Command;

    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       CMD_HELP },
        { "quit",       CMD_QUIT },
        { "solve",      CMD_SOLVE },
        { "",           CMD_RUN_SINGLE },
        { "trials",     CMD_RUN_MULTIPLE },
        { "genpath",    CMD_SET_GENPATH },
    };

    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help               print this help menu"
        "\n- quit               terminate this program"
        "\n- solve <file>       solve the puzzles in <file>"
        "\n- solve <puzzle>     no spaces; zeros mean empty"
        "\n- {enter}            generate a single solution"
        "\n- trials <n>         generate <n> solutions"
        "\n- genpath            cycle generator traversal path"
        ;
    const std::string REPL_PROMPT = "\n$ ";


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
    template <Order O, bool CBT>
    class Repl {
    public:
        explicit Repl(std::ostream&);
        bool runCommand(std::string const& cmdLine);

    private:
        Solver<O,CBT> solver;
        std::ostream& os;

        // Return false if command is to exit the program:
        void solvePuzzlesFromFile(std::ifstream&);
        void runNew(void);
        void runMultiple(const unsigned long);

        void printTrialsWorkDistribution(const trials_t,
            std::array<trials_t, TRIALS_NUM_BINS> const&,
            std::array<double,   TRIALS_NUM_BINS> const&);
    };

}

#endif