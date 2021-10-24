#include <solvent_lib/sort/canon.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/prob.hpp>

#include <iostream>
#include <algorithm> // ranges::sort
#include <numeric>   // accumulate

namespace solvent::lib::canon {

	template<Order O>
	void canonicalize_labels(std::vector<typename size<O>::ord2_t>& input) noexcept {
		// using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		// static constexpr ord4_t O4 = O*O*O*O;
		static constexpr double count_expected = static_cast<double>(2*O2)/(O1+1);
		static constexpr std::array<long, O2+1> O2_choose_r = [](){
			std::array<long, O2+1> arr;
			for (int i = 1; i < 1 + O2/2; i++) {
				arr[O2-i] = arr[i] = n_choose_r(O2, i);
			}
			return arr;
		}();

		/*
		An entry at coordinate (A,B) contains the number of atoms in the
		grid where the values A and B coexist. Rephrase: the number of blocks
		in the grid where A and B are in the same atom.

		The act of choosing two labels A and B and swapping the corresponding
		rows and the corresponding columns produces the table generated when
		instances of A and B in the input grid are swapped.

		The probability of values A and B being in the same atom within a
		block is p(n) = 2/(o+1). The probability of coordinate (A,B) being
		n is given by a binomial distribution B(o^2, 2/(o+1)).

		- The diagonal is all zeroes.
		- Each row (or column) sums to O2*(2*(O1-1)).
		- The maximum possible value at any coordinate is O2.
		- The expected value at any non-diagonal coordinate is 2*O2/(O1+1).
			Simplified from (O2*(2*(O1-1)))/(O2-1). Using law of large numbers.
			Examples by order: 2: 2.67,  3: 4.50,  4: 6.40,  5: 8.33
			It can also be calculated in the standard way by using p(n).
		*/
		std::array<std::array<ord2_t, O2>, O2> counts = {{0}};
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
		struct SortMapEntry {
			ord2_t orig; // The original label value
			double sort_basis = 0; // Relabelling has no effect on this calculated value.
		};
		{
			std::array<SortMapEntry, O2> canon2orig = {};
			// The reduction calculation's result must not depend on the ordering
			// of the counts entries. It should encapsulate a label's degree of
			// preference to being in an atom with some labels more than others.
			// The specific reduction below is the sample standard deviation.
			for (ord2_t label = 0; label < O2; label++) {
				canon2orig[label] = SortMapEntry { .orig = label, .sort_basis = 0 };
				for (ord2_t neighbour_i = 0; neighbour_i < O2; neighbour_i++) {
					if (neighbour_i != label) [[likely]] {
						ord2_t count = counts[label][neighbour_i];

						double p_binomial = static_cast<double>(
							O2_choose_r[count] * std::pow(2, count) * std::pow(O1-1, O2-count)
						) / std::pow(O1+1, O2);

						canon2orig[label].sort_basis += p_binomial * std::pow(
							static_cast<double>(count) - count_expected,
						2);
					}
				}
			}
			std::ranges::stable_sort(canon2orig, {}, &SortMapEntry::sort_basis);
				std::cout << "\n"; for (auto e : canon2orig) { std::cout << e.sort_basis << "  "; }

			std::array<std::array<ord2_t, O2>, O2> input_canon_label;
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					input_canon_label[i][j] = input[(O2 * canon2orig[i].orig) + canon2orig[j].orig];
				}
			}

			// The below only done for debugging purposes.
			decltype(counts) counts_sorted;
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					counts_sorted[i][j] = counts[canon2orig[i].orig][canon2orig[j].orig];
				}
				std::ranges::stable_sort(counts_sorted[i]);
			}
			counts = counts_sorted;
		}
		print::pretty(std::cout, O, grid_accessors);
	}


	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept {
		canonicalize_labels<O>(input);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}