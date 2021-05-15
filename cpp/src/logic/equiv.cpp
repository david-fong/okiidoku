#include "./equiv.hpp"

namespace Sudoku::Equiv {

template <Order O>
void Equiv<O>::normalizeSymbolShuffling() {
	// A coordinate contains the number of horizontal and vertical atoms
	// where the x and y coordinates are values that coexist in an atom.
	// In each block, for some symbol A, there can be up to one horizontal
	// or vertical atoms that contain another symbol B, which is why the
	// element type will fit within value_t.
	std::array<value_t, area> counts = {0};

	for (length_t block = 0; block < length; block++) {
		area_t block_offset = order * ((block % order) + (length * (block / order)));
		for (order_t atom = 0; atom < order; atom++) {
			// Go through all unique pairs in the atom:
			for (order_t atom_i = 0; atom_i < order - 1; atom_i++) {
				for (order_t atom_j = atom_i + 1; atom_j < order; atom_j++) {
					{
						// horizontal atom
						const offset = block_offset + (length * atom);
						const value_t a = grid[offset + atom_i];
						const value_t b = grid[offset + atom_j];
						counts[length * a + b]++;
						counts[length * b + a]++;
					}{
						// vertical atom
						const offset = block_offset + atom;
						const value_t a = grid[offset + (length * atom_i)];
						const value_t b = grid[offset + (length * atom_j)];
						counts[length * a + b]++;
						counts[length * b + a]++;
					}
				}
			}
		}
	}
	// Sort the counts table...
	// Note: in the below struct, sum only needs to hold up to (length*(order-1)*2).
	const std::array<struct { length_t orig_i; area_t sum }, length_t> relabelling;
	for (length_t i = 0; i < length; i++) {
		relabelling[i].orig_i = i;
		const sum = 0;
		for (length_t j; j < length; j++) {
			sum += grid[length * i + j];
		}
	}
}

} // namespace Sudoku::Equiv