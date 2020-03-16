#ifndef HPP_SUDOKU_TRIALS
#define HPP_SUDOKU_TRIALS

#include "solver.hpp"

namespace Sudoku {

typedef unsigned long trials_t;

namespace Trials {

    constexpr unsigned NUM_BINS = 20u;
    enum class StopBy {
        TRIALS,
        SUCCESSES,
    };

    struct SharedState {
        std::mutex&     mutex;
        const unsigned  COLS;
        const StopBy    trialsStopMethod;
        const trials_t  trialsStopThreshold;
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
    template <Order O, bool CBT, GUM::E GUM>
    class ThreadFunc : protected SharedState {
    public:
        using Solver = class Solver<O,CBT,GUM>;
        ThreadFunc(void) = delete;
        ThreadFunc(SharedState s) : SharedState(s) {};
        inline void operator()(Solver* solver, unsigned threadNum);
    };

} // End of Trials namespace
} // End of Sudoku namespace

#endif