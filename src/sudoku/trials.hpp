#ifndef HPP_SUDOKU_TRIALS
#define HPP_SUDOKU_TRIALS

#include "./solver/solver.hpp"

namespace Sudoku {

typedef unsigned long trials_t;

namespace Trials {

    constexpr unsigned NUM_BINS = 20u;
    enum class StopBy : unsigned {
        TRIALS,
        SUCCESSES,
    };

    const std::string TABLE_SEPARATOR = "\n+-----------+----------+----------------+-----------+-----------+";
    const std::string TABLE_HEADER    = "\n|  bin bot  |   hits   |   operations   |  giveup%  |  speedup  |";

    struct SharedState {
        std::mutex&     mutex;
        const unsigned  COLS;
        const Repl::OutputLvl::E outputLvl;
        const StopBy    trialsStopMethod;
        const trials_t  trialsStopThreshold;
        unsigned&       percentDone;
        trials_t&       totalTrials;
        trials_t&       totalSuccesses;
        std::array<trials_t, NUM_BINS+1>& binHitCount;
        std::array<double,   NUM_BINS+1>& binOpsTotal;
    };

    /**
     * A helper for `Repl::runMultiple`.
     * 
     * Note: Since it is only ever used there, the include guards are not
     * absolutely necessary, but it doesn't hurt to add them anyway.
     */
    template <Sudoku::Order O>
    class ThreadFunc final : private SharedState {
      public:
        using solver_t  = class Sudoku::Solver::Solver<O>;
        using OutputLvl = Repl::OutputLvl::E;
      public:
        ThreadFunc(void) = delete;
        explicit ThreadFunc(SharedState s) : SharedState(s) {};
        inline void operator()(solver_t* solver, unsigned threadNum);
      private:
        trials_t trialsStopCurVal(void) const {
            switch (trialsStopMethod) {
                case StopBy::TRIALS:    return totalTrials;
                case StopBy::SUCCESSES: return totalSuccesses;
                default: throw "unhandled enum case";
            }
        }
    };

} // End of Trials namespace
} // End of Sudoku namespace

#endif