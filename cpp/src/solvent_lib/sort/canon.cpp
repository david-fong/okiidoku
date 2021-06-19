#include <solvent_lib/sort/canon.hpp>

#include <algorithm> // ranges::sort
#include <numeric>   // accumulate

namespace solvent::lib::canon {

	template<Order O>
	typename size<O>::ord2_t Canonicalizer<O>::operator[](const ord4_t coord) const {
		return buf_[coord];
	}


	template<Order O>
	void Canonicalizer<O>::handle_relabeling(void) noexcept {
		// An entry at coordinate (R,C) contains the number of atoms in
		// the grid where the values R and C coexist. For each block, this
		// can happen up to one time with probability (2*(O1-1))/(O2-1).
		// Therefore, the maximum value is O2. The diagonal is all zeroes.
		std::array<std::array<ord2_t, O2>, O2> counts = {{}};

		for (ord2_t block = 0; block < O2; block++) {
			ord4_t block_offset = O1 * ((block % O1) + (O2 * (block / O1)));
			for (ord1_t atom = 0; atom < O1; atom++) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{
							// horizontal atom
							const ord4_t offset = block_offset + (O2 * atom);
							const ord2_t a = buf_[offset + atom_i], b = buf_[offset + atom_j];
							counts[a][b]++; counts[b][a]++;
						}{
							// vertical atom
							const ord4_t offset = block_offset + atom;
							const ord2_t a = buf_[offset + (O2 * atom_i)], b = buf_[offset + (O2 * atom_j)];
							counts[a][b]++; counts[b][a]++;
						}
					}
				}
			}
		}
		// Note: in the below struct, sum only needs to hold up to (O2 * (O1-1)*2).
		struct Label {
			ord2_t value;
			ord4_t sum = 0; // Relabelling has no effect on this calculated value.
		};
		const auto sort_by_count_slices = [&](const bool is_tie_breaker) -> void {
			std::array<Label, O2> labels = {};
			for (ord2_t i = 0; i < O2; i++) {
				labels[i] = Label { .value = i, .sum = 0 };
				for (ord2_t j = 0; j < O2; j++) {
					labels[i].sum += !is_tie_breaker ? counts[i][j] : counts[j][i];
				}
			}
			std::ranges::stable_sort(labels, {}, &Label::sum);

			std::array<std::array<ord2_t, O2>, O2> counts_sorted;
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					counts_sorted[i][j] = counts[labels[i].value][labels[j].value];
				}
			}
			counts = counts_sorted;
		};
		sort_by_count_slices(false);
		sort_by_count_slices(true);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}