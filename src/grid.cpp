#include "grid.hpp"

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <ctime>


#define STATW << std::setw(10)
// ^mechanism to statically toggle alignment:


Game::Tile::Tile(const area_t index):
    index       (index),
    biasIndex   (0),
    value       (0),
    fixedVal    (false) {}

Game::Game(const order_t _order, std::ostream& outStream, const bool isPretty):
    order       (CLEAN_ORDER(_order)),
    length      (order  * order),
    area        (length * length),
    outStream   (outStream),
    isPretty    (isPretty)
{
    grid.reserve(area);
    rowBins.resize(length);
    colBins.resize(length);
    blkBins.resize(length);

    for (int i = 0; i < area; i++) {
        grid.emplace_back(i);
    }
    for (length_t i = 0; i < length; i++) {
        std::vector<value_t> seq;
        for (value_t i = 0; i <= length; i++) {
            seq.push_back(i);
        }
        rowBiases.push_back(std::move(seq));
    }
    totalGenCount = 0;
    successfulGenCount = 0;

    // Output formatting:
    if (isPretty) {
        outStream.imbue(std::locale(outStream.getloc(), new MyNumpunct()));
    }
    outStream.precision(3);
    outStream << std::fixed;
}

void Game::print(void) const {
    // TODO: handle pretty-print style.
    outStream << std::setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            outStream << "\n";
        }
        Tile const& t = grid.at(i);
        if (isClear(t)) {
            outStream << "  ";
        } else {
            outStream << ((t.fixedVal) ? "." : " ");
            outStream << (uint16_t)t.value;
        }
    }
    outStream << std::setbase(10) << std::endl;
}


void Game::clear(void) {
    // Initialize all values as empty:
    std::for_each(grid.begin(), grid.end(), [this](Tile& t){ t.clear(length); });
    std::fill(rowBins.begin(), rowBins.end(), 0);
    std::fill(colBins.begin(), colBins.end(), 0);
    std::fill(blkBins.begin(), blkBins.end(), 0);
}

void Game::seed(const bool printInfo) {
    // Scramble each row's value-guessing-order:
    // note: must keep the <length>'th entry as <length>.
    for (auto& rowBias : rowBiases) {
        std::random_shuffle(rowBias.begin(), rowBias.end() - 1, myRandom);
    }
    // Call the seeding routines:
    const area_t seed0Seeds = seed0();
    const area_t seed1Seeds = seed1(order + seed1Constants[order]);
    if (printInfo) {
        outStream << "stage 01 seeds: " STATW << seed0Seeds << std::endl;
        outStream << "stage 02 seeds: " STATW << seed1Seeds << std::endl;
    }
    // std::sort(grid.begin(), grid.end(),
    //         [this](Tile const& t1, Tile const& t2) -> bool {
    //     return tileNumCandidates(t1.index) < tileNumCandidates(t2.index);
    // });
}


opcount_t Game::generateSolution(void) {
    opcount_t giveupThreshold = GIVEUP_RATIO * area * area;
    opcount_t numOperations = 0;
    std::vector<Tile>::iterator it = grid.begin();
    // Skip all seeded starting tiles:
    while (it->fixedVal && it < grid.end()) it++;

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
                // Fail if no solution could be found:
                if (it == grid.begin()) { throw Game::OPseed; }
            } while ((--it)->fixedVal);
        } else {
            // Step forward to push a new permutation:
            while (++it < grid.end() && it->fixedVal);
        }
    }
    totalGenCount++;
    successfulGenCount++;
    return numOperations;
}

Game::Tile& Game::setNextValid(const area_t index) {
    occmask_t& rowBin = rowBins[getRow(index)];
    occmask_t& colBin = colBins[getCol(index)];
    occmask_t& blkBin = blkBins[getBlk(index)];

    Tile& t = grid[index];
    if (!isClear(t)) {
        // If the tile is currently already set, clear it:
        const occmask_t eraseMask = ~(0b1 << t.value);
        rowBin &= eraseMask;
        colBin &= eraseMask;
        blkBin &= eraseMask;
    }
    // default value:
    t.value = length;

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

length_t Game::tileNumCandidates(const area_t index) const {
    occmask_t unoccMask = ~(rowBins[getRow(index)]
        | colBins[getCol(index)]
        | blkBins[getBlk(index)]);
    value_t numCandidates = 0;
    for (length_t i = 0; i < length; i++) {
        if (unoccMask & 0b1) numCandidates++;
        unoccMask >>= 1;
    }
    return numCandidates;
}





bool Game::runCommand(std::string const& cmdLine) {
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
        default:
            break; // unreachable.
    }
    return true;
}

void Game::runNew(void) {
    printMessageBar("START " + std::to_string(totalGenCount));
    clear();
    seed(true);

    // Generate a new solution:
    const clock_t clockStart    = clock();
    const opcount_t numSolveOps = generateSolution();
    const clock_t clockFinish   = clock();
    const double processorTime  = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;
    outStream << "num operations: " STATW << numSolveOps   << std::endl;
    outStream << "processor secs: " STATW << processorTime << std::endl;

    // Print out the grid:
    std::sort(grid.begin(), grid.end());
    print();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}

void Game::runMultiple(unsigned int numAttempts) {
#define PRINT_COLS 8
    const opcount_t initialGenCount = totalGenCount;
    outStream << std::endl;
    printMessageBar("START x" + std::to_string(numAttempts));
    for (unsigned int attemptNum = 0; attemptNum < numAttempts; attemptNum++) {
        clear();
        seed(false);
        const opcount_t numSolveOps = generateSolution();
        // No need to sort grid back into order: No content print.
        if (numSolveOps == 0) {
            outStream STATW << "---";
        } else {
            outStream STATW << numSolveOps;
        }
        if ((totalGenCount - initialGenCount) % PRINT_COLS == 0) {
            outStream << std::endl;
        }
    }
    printMessageBar("DONE x" + std::to_string(numAttempts));
}





area_t Game::seed0(void) {
    if (order <= 2) return 0; // overpowered.
    area_t count = 0;
    const area_t bRow = order * length;
    for (area_t b = 0; b < area; b += bRow + order) {
        for (area_t r = 0; r < bRow; r += length) {
            for (order_t c = 0; c < order; c++) {
                setNextValid(b + r + c).fixedVal = true;
                count++;
            }
        }
    }
    return count;
}

bool Game::seed1Bitmask(const area_t index, const occmask_t min) {
    occmask_t& row = rowBins[getRow(index)];
    occmask_t& col = colBins[getCol(index)];
    occmask_t& blk = blkBins[getBlk(index)];

    if ((row | col | blk) < min) {
        // seed this tile:
        grid[index].fixedVal = true;
        row = (row << 1) | 0b1;
        col = (col << 1) | 0b1;
        blk = (blk << 1) | 0b1;
        return true;
    }
    return false;
}

area_t Game::seed1(int ceiling) {
    if (order <= 2) return 0; // overpowered.
    ceiling = ~0 << (ceiling);
    Game occ(order, outStream, false);
    occ.clear(); // no seeding required.
    for (area_t i = 0; i < area; i++) {
        if (grid[i].fixedVal) {
            occ.seed1Bitmask(i, ~0);
        }
    }

    area_t i = -1;
    // Scan forward past those already seeded.
    while (occ.grid[++i].fixedVal);
    occmask_t min = (
        occ.rowBins[getRow(i)] |
        occ.colBins[getCol(i)] |
        occ.blkBins[getBlk(i)] );

    while (!(min & ceiling)) {
        occ.seed1Bitmask(i, min);

        // Advance and wrap-around if necessary:
        while (++i < area && occ.grid[i].fixedVal);
        if (i == area) {
            i = 0; min = (min << 1) | 0b1;
            while (occ.grid[++i].fixedVal);
        }
    }

    area_t count = 0;
    for (area_t i = 0; i < area; i++) {
        if (occ.grid[i].fixedVal && !grid[i].fixedVal) {
            setNextValid(i).fixedVal = true;
            count++;
        }
    }
    return count;
}



void Game::printMessageBar(std::string const& msg) const {
    unsigned long long barLength = (isPretty)
        ? ((length + order + 1) * 2)
        : (length * 2);
    if (barLength < msg.length() + 8) {
        barLength = msg.length() + 8;
    }
    std::string bar(barLength, '=');
    if (!msg.empty()) {
        bar.replace(4, msg.length(), msg);
        bar.at(3) = ' ';
        bar.at(4 + msg.length()) = ' ';
    }
    outStream << bar << std::endl;
}

