#include "./mod.hpp"

namespace solvent::lib::canon {

template <Order O>
void Canonicalizer<O>::normalizeSymbolShuffling(void) {
	// A coordinate contains the number of horizontal and vertical atoms
	// where the x and y coordinates are values that coexist in an atom.
	// In each block, for some symbol A, there can be up to one horizontal
	// or vertical atoms that contain another symbol B, which is why the
	// element type will fit within value_t.
	std::array<value_t, O4> counts = {0};

	for (length_t block = 0; block < O2; block++) {
		area_t block_offset = O1 * ((block % O1) + (O2 * (block / O1)));
		for (order_t atom = 0; atom < O1; atom++) {
			// Go through all unique pairs in the atom:
			for (order_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
				for (order_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
					{
						// horizontal atom
						const offset = block_offset + (O2 * atom);
						const value_t a = buf[offset + atom_i];
						const value_t b = buf[offset + atom_j];
						counts[O2 * a + b]++;
						counts[O2 * b + a]++;
					}{
						// vertical atom
						const offset = block_offset + atom;
						const value_t a = buf[offset + (O2 * atom_i)];
						const value_t b = buf[offset + (O2 * atom_j)];
						counts[O2 * a + b]++;
						counts[O2 * b + a]++;
					}
				}
			}
		}
	}
	// Sort the counts table...
	// Note: in the below struct, sum only needs to hold up to (O2*(O1-1)*2).
	const std::array<struct { length_t orig_i; area_t sum }, length_t> relabelling;
	for (length_t i = 0; i < O2; i++) {
		relabelling[i].orig_i = i;
		const sum = 0;
		for (length_t j; j < O2; j++) {
			sum += grid[O2 * i + j];
		}
	}
}

}