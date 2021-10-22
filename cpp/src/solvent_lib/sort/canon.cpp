#include <solvent_lib/sort/canon.hpp>
#include <solvent_lib/print.hpp>

#include <iostream>
#include <algorithm> // ranges::sort
#include <numeric>   // accumulate

namespace solvent::lib::canon {

	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept {
		// using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t  = typename size<O>::ord1_t;
		using ord2_t  = typename size<O>::ord2_t;
		using ord4_t  = typename size<O>::ord4_t;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		// static constexpr ord4_t O4 = O*O*O*O;

		// An entry at coordinate (A,B) contains the number of atoms in
		// the grid where the values A and B coexist. For each block, this
		// can happen up to one time with probability (2*(O1-1))/(O2-1)
		// (choose an arbitrary coordinate in the block as A, and of the
		// remaining O2-1 coordinates, 2*(O1-1) are in the same atom).
		// The maximum value at any coordinate is is O2. The diagonal is
		// all zeroes. Every row and column sums to O2*(2*(O1-1)).
		std::array<std::array<ord2_t, O2>, O2> counts = {{}};
		const std::vector<print::print_grid_t> grid_accessors = {
			print::print_grid_t([&counts](std::ostream& os, uint16_t coord) {
				os << ' '; print::val2str(os, O, counts[coord/O2][coord%O2]);
			}),
		};

		for (ord2_t block = 0; block < O2; block++) {
			ord4_t block_offset = O1 * ((O2 * (block / O1)) + (block % O1));
			for (ord1_t atom = 0; atom < O1; atom++) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{
							// horizontal atom
							const ord4_t offset = block_offset + (O2 * atom);
							const ord2_t a = input[offset + atom_i], b = input[offset + atom_j];
							counts[a][b]++; counts[b][a]++;
						}{
							// vertical atom
							const ord4_t offset = block_offset + atom;
							const ord2_t a = input[offset + (O2 * atom_i)], b = input[offset + (O2 * atom_j)];
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

		// TODO this section deprecated. I realized the algorithm hereon doesn't work.
		print::pretty(std::cout, O, grid_accessors);
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

		print::pretty(std::cout, O, grid_accessors);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}