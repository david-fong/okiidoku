#include <solvent_lib/sort/canon.hpp>

namespace solvent::lib::canon {

	template<Order O>
	void Canonicalizer<O>::handle_relabeling(void) noexcept {
		// A coordinate contains the number of horizontal and vertical atoms
		// where the x and y coordinates are values that coexist in an atom.
		// In each block, for some symbol A, there can be up to one horizontal
		// or vertical atoms that contain another symbol B, which is why the
		// element type will fit within ord2_t.
		std::array<ord2_t, O4> counts = {0};

		for (ord2_t block = 0; block < O2; block++) {
			ord4_t block_offset = O1 * ((block % O1) + (O2 * (block / O1)));
			for (ord1_t atom = 0; atom < O1; atom++) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{
							// horizontal atom
							const offset = block_offset + (O2 * atom);
							const ord2_t a = buf[offset + atom_i];
							const ord2_t b = buf[offset + atom_j];
							counts[O2 * a + b]++;
							counts[O2 * b + a]++;
						}{
							// vertical atom
							const offset = block_offset + atom;
							const ord2_t a = buf[offset + (O2 * atom_i)];
							const ord2_t b = buf[offset + (O2 * atom_j)];
							counts[O2 * a + b]++;
							counts[O2 * b + a]++;
						}
					}
				}
			}
		}
		// Sort the counts table...
		// Note: in the below struct, sum only needs to hold up to (O2*(O1-1)*2).
		const std::array<struct { ord2_t orig_i; ord4_t sum }, ord2_t> relabelling;
		for (ord2_t i = 0; i < O2; i++) {
			relabelling[i].orig_i = i;
			const sum = 0;
			for (ord2_t j; j < O2; j++) {
				sum += buf[O2 * i + j];
			}
		}
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}