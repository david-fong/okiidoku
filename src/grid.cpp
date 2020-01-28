#include <iomanip>

#include "grid.h"

Game::Tile::Tile():
    fixedVal    (false),
    biasIndex   (0),
    value       (0) {}

Game::Tile::Tile(const value_t rowLen):
    fixedVal    (false),
    biasIndex   (rowLen),
    value       (rowLen) {}

Game::Game(const order_t _order):
    order   (CLEAN_ORDER(_order)),
    length  (order  * order),
    area    (length * length)
{
    grid    = vector<Tile>(area);
    rowBins = vector<occmask_t>(length);
    colBins = vector<occmask_t>(length);
    blkBins = vector<occmask_t>(length);

    for (length_t i = 0; i < length; i++) {
        vector<value_t> seq;
        for (value_t i = 0; i <= length; i++) seq.push_back(i);
        rowBiases.push_back(seq);
    }
}

void Game::clear() {
    // Initialize all values as empty:
    // TODO: fill new Tiles in ctor, call new Tile::clear method here.
    fill(grid.begin(), grid.end(), Tile(length));
    fill(rowBins.begin(), rowBins.end(), 0);
    fill(colBins.begin(), colBins.end(), 0);
    fill(blkBins.begin(), blkBins.end(), 0);
}

void Game::runNew() {
    clear();
    // Scramble each row's value-guessing-order:
    // *set the <length>'th entry to <length>
    for (length_t i = 0; i < length; i++) {
        random_shuffle(rowBiases[i].begin(), rowBiases[i].end() - 1, myRandom);
    }
    // Generate a solution:
    SEED0();
    cout << "@ Stage 2: Seeds added: " << seed1(order + seed1Constants[order]);
    generateSolution();
    // When done, set all values as fixed:
    for (int i = 0; i < area; i++) {
        grid[i].fixedVal = true;
    }
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

void Game::generateSolution() {
    area_t i = 0;
    // Skip all seeded starting tiles:
    while (grid[i].fixedVal && i < area) i++;

    while (i < area) {
        // Push a new permutation:
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
}

void Game::print(ostream& out) const {
    out << setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0) {
            out << "\n";
        }
        const Tile& t = grid[i];
        if (isClear(t)) {
            out << "  ";
        } else {
            out  << ((t.fixedVal) ? "." : " ");
            out << (uint16_t)t.value;
        }
    }
    out << setbase(10) << endl;
}












void Game::seed0() {
    const area_t bRow = order * length;
    for (area_t b = 0; b < area; b += bRow + order)
        for (area_t r = 0; r < bRow; r += length)
            for (order_t c = 0; c < order; c++)
                setNextValid(b + r + c).fixedVal = true;
}

void Game::seed0b() {
    // Get offsets to blocks along top and left edge:
    vector<area_t> blocks;
    const area_t bRow = order * length;
    for (length_t i = 0; i < length; i += order)
        blocks.push_back(i);
    for (area_t i = bRow; i < area; i += bRow)
        blocks.push_back(i);

    for (length_t i = 0; i < blocks.size(); i++)
        for (area_t r = 0; r < bRow; r += length)
            for (order_t c = 0; c < order; c++)
                setNextValid(blocks[i] + r + c).fixedVal = true;
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
    ceiling = ~0 << (ceiling);
    Game occ(order);
    occ.clear();
    for (area_t i = 0; i < area; i++) {
        if (grid[i].fixedVal) { occ.seed1Bitmask(i, ~0); }
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
for (area_t i = 0; i < area; i++) {
    if (occ.grid[i].fixedVal && !grid[i].fixedVal) {
        setNextValid(i).fixedVal = true;
    }
}
print(cout);
            i = 0; min = (min << 1) | 0b1;
            while (occ.grid[++i].fixedVal);
        }
    }

    area_t count = 0;
    for (area_t i = 0; i < area; i++) {
        if (occ.grid[i].fixedVal && !grid[i].fixedVal) {
            count++;
            //setNextValid(i).fixedVal = true;
        }
    }
    return count;
}

