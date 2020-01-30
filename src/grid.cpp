#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>
#include <ctime>

#include "grid.h"


Game::Tile::Tile(const area_t index):
    index       (index),
    biasIndex   (0),
    value       (0),
    fixedVal    (false) {}

Game::Game(const order_t _order, ostream& outStream, const bool isPretty):
    order       (CLEAN_ORDER(_order)),
    length      (order  * order),
    area        (length * length),
    outStream   (outStream),
    isPretty    (isPretty)
{
    grid    = vector<Tile>();
    rowBins = vector<occmask_t>(length);
    colBins = vector<occmask_t>(length);
    blkBins = vector<occmask_t>(length);

    for (area_t i = 0; i < area; i++) {
        grid.push_back(Tile(i));
    }
    for (length_t i = 0; i < length; i++) {
        vector<value_t> seq;
        for (value_t i = 0; i <= length; i++) {
            seq.push_back(i);
        }
        rowBiases.push_back(seq);
    }
    totalGenCount = 0;
    successfulGenCount = 0;

    // Output formatting:
    if (isPretty) {
        outStream.imbue(locale(outStream.getloc(), new MyNumpunct()));
    }
    outStream.precision(3);
    outStream << fixed;
}

void Game::print(void) const {
    // TODO: handle pretty-print style.
    outStream << setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            outStream << "\n";
        }
        const Tile& t = grid[i];
        if (isClear(t)) {
            outStream << "  ";
        } else {
            outStream << ((t.fixedVal) ? "." : " ");
            outStream << (uint16_t)t.value;
        }
    }
    outStream << setbase(10) << endl;
}

void Game::clear(const bool printSeedInfo) {
    // Initialize all values as empty:
    for_each(grid.begin(), grid.end(), [this](Tile& t){ t.clear(length); });
    fill(rowBins.begin(), rowBins.end(), 0);
    fill(colBins.begin(), colBins.end(), 0);
    fill(blkBins.begin(), blkBins.end(), 0);

    // Scramble each row's value-guessing-order:
    // note: must keep the <length>'th entry as <length>.
    for (length_t i = 0; i < length; i++) {
        random_shuffle(rowBiases[i].begin(), rowBiases[i].end() - 1, myRandom);
    }
    // Call the seeding routines:
    const area_t seed0Seeds = seed0();
    const area_t seed1Seeds = seed1(order + seed1Constants[order]);
    if (printSeedInfo) {
        outStream << "stage 01 seeds: " STATW << seed0Seeds << endl;
        outStream << "stage 02 seeds: " STATW << seed1Seeds << endl;
    }
    traversalPath = vector<Tile&>();
    // TODO: ^ initialize with all non-seeded Tiles sorted descending by num-candidates.
}

opcount_t Game::generateSolution(void) {
    opcount_t giveupThreshold = GIVEUP_RATIO * area * area;
    opcount_t numOperations = 0;
    area_t i = 0;
    // Skip all seeded starting tiles:
    while (grid[i].fixedVal && i < area) i++;

    while (i < area) {
        // Push a new permutation:
        numOperations++;
        if (numOperations > giveupThreshold) {
            totalGenCount++;
            return 0;
        }
        if (isClear(setNextValid(i))) {
            // Pop and step backward:
            do {
                // Fail if no solution could be found:
                if (i == 0) { throw Game::OPseed; }
            } while (grid[--i].fixedVal);
        } else {
            // Step forward to push a new permutation:
            while (++i < area && grid[i].fixedVal);
        }
    }
    totalGenCount++;
    successfulGenCount++;
    return numOperations;
}

Game::Tile& Game::setNextValid(const area_t index) {
    const length_t row = getRow(index);
    occmask_t& colBin = colBins[getCol(index)];
    occmask_t& blkBin = blkBins[getBlk(index)];

    Tile& t = grid[index];
    if (!isClear(t)) {
        // If the tile is currently already set, clear it:
        const occmask_t eraseMask = ~(0b1 << t.value);
        rowBins[row] &= eraseMask;
        colBin &= eraseMask;
        blkBin &= eraseMask;
    }
    // descriptive (but not meaningful, according to spec) default value:
    t.value = length;

    occmask_t invalidBin = rowBins[row] | colBin | blkBin;
    value_t biasIndex = (t.biasIndex + 1) % (length + 1);
    for (; biasIndex < length; biasIndex++) {
        value_t value = rowBiases[row][biasIndex];
        const occmask_t valBit = 0b1 << value;
        if (!(invalidBin & valBit)) {
            // If a valid value is found for this tile:
            rowBins[row] |= valBit;
            colBin |= valBit;
            blkBin |= valBit;
            t.value = value;
            break;
        }
    }
    t.biasIndex = biasIndex;
    return t;
}





bool Game::runCommand(const string& cmdLine) {
    // purposely use cout instead of this.outStream.
    size_t tokenPos;
    const string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const string cmdArgs = cmdLine.substr(tokenPos + 1, string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched
        cout << "command not found. enter \"help\" for the help menu." << endl;
        return true;
    }
    switch (it->second) {
        case HELP:
            cout << HELP_MESSAGE << endl;
            break;
        case QUIT:
            return false;
        case RUN_SINGLE:
            runNew();
            break;
        case RUN_MULTIPLE:
            try {
                runMultiple(stoi(cmdArgs));
            } catch (const invalid_argument& ia) {
                cout << "could not convert " << cmdArgs << " to an integer." << endl;
            }
            break;
        default:
            break; // unreachable.
    }
    return true;
}

void Game::runNew(void) {
#define STATW << setw(10)
// ^mechanism to statically toggle alignment:
    printMessageBar("START " + to_string(totalGenCount));
    clear(true);

    // Generate a new solution:
    const clock_t clockStart    = clock();
    const opcount_t numSolveOps = generateSolution();
    const clock_t clockFinish   = clock();
    const double processorTime  = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;
    outStream << "num operations: " STATW << numSolveOps << endl;
    outStream << "processor secs: " STATW << processorTime << endl;

    // Print out the grid:
    print();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}

void Game::runMultiple(unsigned int numAttempts) {
#define PRINT_COLS 8
#define STATW << setw(10)
// ^mechanism to statically toggle alignment:
    const opcount_t initialGenCount = totalGenCount;
    outStream << endl;
    printMessageBar("START x" + to_string(numAttempts));
    for (unsigned int attemptNum = 0; attemptNum < numAttempts; attemptNum++) {
        clear(false);
        const opcount_t numSolveOps = generateSolution();
        if (numSolveOps == 0) {
            outStream STATW << "---";
        } else {
            outStream STATW << numSolveOps;
        }
        if ((totalGenCount - initialGenCount) % PRINT_COLS == 0) {
            outStream << endl;
        }
    }
    printMessageBar("DONE x" + to_string(numAttempts));
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
    occ.clear();
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



void Game::printMessageBar(const string& msg) const {
    unsigned long long barLength = (isPretty)
        ? ((length + order + 1) * 2)
        : (length * 2);
    if (barLength < msg.length() + 8) {
        barLength = msg.length() + 8;
    }
    string bar(barLength, '=');
    if (!msg.empty()) {
        bar.replace(4, msg.length(), msg);
        bar.at(3) = ' ';
        bar.at(4 + msg.length()) = ' ';
    }
    outStream << bar << endl;
}

