#include "grid.hpp"

#include <iostream>     // cout,
#include <iomanip>      // setbase, setw,
#include <ctime>        // clock,
#include <numeric>      // iota,
#include <algorithm>    // sort, random_shuffle,
#include <string>       // string,
#include <bitset>       // bitset,


#define STATW_I << std::setw(this->statsWidth)
#define STATW_D << std::setw(this->statsWidth + 4)
// ^mechanism to statically toggle alignment:

#define TRAVERSE_BY_BOTTLENECK false


Sudoku::Sudoku(const order_t _order, std::ostream& outStream):
    order       (CLEAN_ORDER(_order)),
    length      (order  * order),
    area        (length * length),
    outStream   (outStream),
    isPretty    (&outStream == &std::cout),
    statsWidth  ((0.5 * length) + 3)
{
    grid.reserve(area);
    seeds.resize(area, false);
    rowBins.resize(length);
    colBins.resize(length);
    blkBins.resize(length);

    for (int i = 0; i < area; i++) {
        // Construct a new Tile with index `i` at the back.
        grid.emplace_back(i);
    }
    rowBiases.reserve(length);
    for (length_t i = 0; i < length; i++) {
        rowBiases.emplace_back(length + 1);
        std::iota(rowBiases[i].begin(), rowBiases[i].end(), 0);
    }
    totalGenCount = 0;
    successfulGenCount = 0;
    doSeeding = false; // it's faster :O

    // Output formatting:
    if (isPretty) {
        outStream.imbue(std::locale(outStream.getloc(), new MyNumpunct()));
    }
    outStream.precision(3);
    outStream << std::fixed;
}

void Sudoku::print(void) const {
    static std::string vbar = ' ' + std::string(((length + order + 1) * 2 - 1), '-');
    for (int i = 0; i <= order; i++) {
        vbar[1 + i * 2 * (order + 1)] = '+';
    }
    outStream << std::setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            if (isPretty) outStream << " |";
            outStream << "\n";
        }
        if (isPretty && (i % (order * length) == 0)) {
            outStream << vbar << '\n';
        }
        if (isPretty && (i % order) == 0) outStream << " |";
        Tile const& t = grid.at(i);
        if (isClear(t)) {
            outStream << "  ";
        } else {
            outStream << ((seeds[i]) ? "." : " ");
            if (order < 5) {
                outStream << (uint16_t)t.value;
            } else {
                outStream << (char)('a' + t.value);
            }
        }
    }
    if (isPretty) {
        outStream << " |\n" << vbar;
    }
    outStream << std::setbase(10) << std::endl;
}


void Sudoku::clear(void) {
    // Initialize all values as empty:
    std::for_each(grid.begin(), grid.end(), [this](Tile& t){ t.clear(length); });
    std::fill(seeds.begin(), seeds.end(), false);
    std::fill(rowBins.begin(), rowBins.end(), 0);
    std::fill(colBins.begin(), colBins.end(), 0);
    std::fill(blkBins.begin(), blkBins.end(), 0);
}

void Sudoku::seed(const bool printInfo) {
    // Scramble each row's value-guessing-order:
    // note: must keep the <length>'th entry as <length>.
    for (auto& rowBias : rowBiases) {
        std::random_shuffle(rowBias.begin(), rowBias.end() - 1, myRandom);
    }
    // Call the seeding routines:
    if (!doSeeding) return;
    const area_t seed0Seeds = seed0();
    const area_t seed1Seeds = seed1(seed1Constants[order]);
    if (printInfo) {
        outStream << "stage 01 seeds: " STATW_I << seed0Seeds << std::endl;
        outStream << "stage 02 seeds: " STATW_I << seed1Seeds << std::endl;
    }
#if TRAVERSE_BY_BOTTLENECK == true
    std::sort(grid.begin(), grid.end(),
            [this](Tile const& t1, Tile const& t2) -> bool {
        return tileNumNonCandidates(t1.index) < tileNumNonCandidates(t2.index);
    });
#endif
}


opcount_t Sudoku::generateSolution(void) {
    static const opcount_t giveupThreshold = GIVEUP_THRESH_COEFF * (area * area * area);
    opcount_t numOperations = 0;
    std::vector<Tile>::iterator it = grid.begin();
    // Skip all seeded starting tiles:
    while (seeds[it->index] && it < grid.end()) it++;
    while (it < grid.end()) {
        // Push a new permutation:
        numOperations++;
        if (numOperations > giveupThreshold) {
            totalGenCount++;
            return 0;
        }
        if (isClear(setNextValid(it->index))) {
            // Pop and step backward:
            do {
                if (it == grid.begin()) { throw Sudoku::OPseed; }
            } while (seeds[(--it)->index]);
        } else {
            while (++it < grid.end() && seeds[it->index]);
        }
    }
    totalGenCount++;
    successfulGenCount++;
    return numOperations;
}

Sudoku::Tile& Sudoku::setNextValid(const area_t index) {
    occmask_t& rowBin = rowBins[getRow(index)];
    occmask_t& colBin = colBins[getCol(index)];
    occmask_t& blkBin = blkBins[getBlk(index)];
    //__builtin_ffsl(0); // TODO: investigate ways to use this for performance gains.

    Tile& t = grid[index];
    if (!isClear(t)) {
        // If the tile is currently already set, clear it:
        const occmask_t eraseMask = ~(0b1 << t.value);
        rowBin &= eraseMask;
        colBin &= eraseMask;
        blkBin &= eraseMask;
    }
    // default value:
    //t.value = length;

    const occmask_t invalidBin = rowBin | colBin | blkBin;
    value_t biasIndex = (t.biasIndex + 1) % (length + 1);
    for (; biasIndex < length; biasIndex++) {
        value_t value = rowBiases[getRow(index)][biasIndex];
        const occmask_t valBit = 0b1 << value;
        if (!(invalidBin & valBit)) {
            // If a valid value is found for this tile:
            rowBin |= valBit;
            colBin |= valBit;
            blkBin |= valBit;
            t.value = value;
            break;
        }
    }
    t.biasIndex = biasIndex;
    return t;
}

length_t Sudoku::tileNumNonCandidates(const area_t index) const {
    return __builtin_popcount(
          rowBins[getRow(index)]
        | colBins[getCol(index)]
        | blkBins[getBlk(index)]
    );
}





bool Sudoku::runCommand(std::string const& cmdLine) {
    // purposely use cout instead of this.outStream.
    size_t tokenPos;
    const std::string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const std::string cmdArgs = cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched
        std::cout << "command not found. enter \"help\" for the help menu." << std::endl;
        return true;
    }
    switch (it->second) {
        case HELP:
            std::cout << HELP_MESSAGE << std::endl;
            break;
        case QUIT:
            return false;
        case RUN_SINGLE:
            runNew();
            break;
        case RUN_MULTIPLE:
            try {
                runMultiple(std::stoi(cmdArgs));
            } catch (std::invalid_argument const& ia) {
                std::cout << "could not convert " << cmdArgs << " to an integer." << std::endl;
            }
            break;
        case TOGGLE_SEEDING: {
            doSeeding = !doSeeding;
            std::cout << "seeding is now " << ((doSeeding) ? "on" : "off") << std::endl;
            break; }
        default:
            break; // unreachable.
    }
    return true;
}

void Sudoku::runNew(void) {
    printMessageBar("START " + std::to_string(totalGenCount));
    clear();
    seed(true);

    // Generate a new solution:
    const clock_t clockStart    = clock();
    const opcount_t numSolveOps = generateSolution();
    const clock_t clockFinish   = clock();
    const double processorTime  = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;
    outStream << "num operations: " STATW_I << numSolveOps   << std::endl;
    outStream << "processor secs: " STATW_D << processorTime << std::endl;
    if (!isPretty) printMessageBar("", '-');

    // Print out the grid:
#if TRAVERSE_BY_BOTTLENECK == true
    std::sort(grid.begin(), grid.end());
#endif
    print();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}

void Sudoku::runMultiple(const unsigned int numAttempts) {
    static const unsigned PRINT_COLS = ((unsigned[]){0,0,16,12,8,1})[order];
    unsigned long totalNumTrials = 0;
    unsigned long successfulNumTrials = 0;
    double totalSuccessfulOperationCount = 0.0;
    printMessageBar("START x" + std::to_string(numAttempts), (statsWidth * PRINT_COLS));

    for (unsigned int attemptNum = 0; attemptNum < numAttempts; attemptNum++) {
        clear();
        seed(false);
        const opcount_t numSolveOps = generateSolution();
        totalNumTrials++;
        // No need to sort grid back into order: No content print.
        if (numSolveOps == 0) {
            outStream STATW_I << "---";
        } else {
            successfulNumTrials++;
            totalSuccessfulOperationCount += numSolveOps;
            outStream STATW_I << numSolveOps;
        }
        if (totalNumTrials % PRINT_COLS == 0) {
            outStream << std::endl;
        }
    }
    if (totalNumTrials % PRINT_COLS != 0) {
        // Final newline:
        outStream << std::endl;
    }

    // Print stats:
    const double averageNumSolveOps = (successfulNumTrials == 0) ? 0.0
        : ((double)totalSuccessfulOperationCount / successfulNumTrials);
    printMessageBar("", (statsWidth * PRINT_COLS), '-');
    outStream << "avg num operations" STATW_D << averageNumSolveOps << std::endl;
    outStream << "num aborted trials" STATW_I << (totalNumTrials - successfulNumTrials) << std::endl;
    printMessageBar("DONE x" + std::to_string(numAttempts), (statsWidth * PRINT_COLS));

    // If printing to file and some time has passed, emit a beep sound:
    if (&outStream != &std::cout) {
        // TODO: count processor time please.
        std::cout << '\a' << std::flush;
    }
}




area_t Sudoku::seed0(void) {
    if (order <= 2) return 0; // overpowered.
    area_t count = 0;
    static const area_t bRow = order * length;
    for (area_t b = 0; b < area; b += bRow + order) {
        for (area_t r = 0; r < bRow; r += length) {
            for (order_t c = 0; c < order; c++) {
                const area_t index = b + r + c;
                setNextValid(index);
                seeds[index] = true;
                count++;
            }
        }
    }
    return count;
}

bool Sudoku::seed1Bitmask(const area_t index, const occmask_t min) {
    occmask_t& row = rowBins[getRow(index)];
    occmask_t& col = colBins[getCol(index)];
    occmask_t& blk = blkBins[getBlk(index)];

    if ((row | col | blk) < min) {
        // seed this tile:
        seeds[index] = true;
        row = (row << 1) | 0b1;
        col = (col << 1) | 0b1;
        blk = (blk << 1) | 0b1;
        return true;
    }
    return false;
}

area_t Sudoku::seed1(int ceiling) {
    ceiling = ~0 << (ceiling);
    static Sudoku occ(order, outStream);
    occ.clear(); // no seeding required.
    for (area_t i = 0; i < area; i++) {
        if (seeds[i]) {
            occ.seed1Bitmask(i, ~0);
        }
    }

    area_t i = -1;
    // Scan forward past those already seeded.
    while (occ.seeds[++i]);
    occmask_t min = (
        occ.rowBins[getRow(i)] |
        occ.colBins[getCol(i)] |
        occ.blkBins[getBlk(i)] );

    while (!(min & ceiling)) {
        occ.seed1Bitmask(i, min);

        // Advance and wrap-around if necessary:
        while (++i < area && occ.seeds[i]);
        if (i == area) {
            i = 0; min = (min << 1) | 0b1;
            while (occ.seeds[++i]);
        }
    }

    area_t count = 0;
    for (area_t i = 0; i < area; i++) {
        if (occ.seeds[i] && !seeds[i]) {
            setNextValid(i);
            seeds[i] = true;
            count++;
        }
    }
    return count;
}



void Sudoku::printMessageBar(
    std::string const& msg,
    unsigned int barLength,
    const char fillChar
) const {
    if (barLength < msg.length() + 8) {
        barLength = msg.length() + 8;
    }
    std::string bar(barLength, fillChar);
    if (!msg.empty()) {
        bar.replace(4, msg.length(), msg);
        bar.at(3) = ' ';
        bar.at(4 + msg.length()) = ' ';
    }
    outStream << bar << std::endl;
}

void Sudoku::printMessageBar(
    std::string const& msg,
    const char fillChar
) const {
    const unsigned int barLength = (isPretty)
        ? ((length + order + 1) * 2)
        : (length * 2);
    return printMessageBar(msg, barLength, fillChar);
}
