#include <solvent_lib/sort/canon.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/prob.hpp>

#include <iostream>
#include <algorithm> // ranges::sort, ranges::greater
#include <array>
#include <cmath>     // pow

namespace solvent::lib::canon {

	template<Order O>
	void Canonicalizer<O>::relabel_(void) noexcept {

		/*
		An entry at coordinate (A,B) contains the number of atoms in the
		grid where the values A and B coexist. Rephrase: the number of blocks
		in the grid where A and B are in the same atom.

		Choosing two labels A and B and swapping the corresponding rows and
		the corresponding columns produces the table generated when instances
		of A and B in `input_` are swapped.

		The probability of values A and B being in the same atom within a
		block is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of coordinate (A,B) being n is given by a binomial
		distribution B(o^2, 2/(o+1)).

		- The diagonal is all zeroes.
		- Each row (or column) sums to O2*(2*(O1-1)).
		- The maximum possible value at any coordinate is O2.
		- The expected value at any non-diagonal coordinate is 2*O2/(O1+1).
			Simplified from (O2*(2*(O1-1)))/(O2-1). Using law of large numbers.
			Examples by order: 2: 2.67,  3: 4.50,  4: 6.40,  5: 8.33
			It can also be calculated in the standard way by using p(n).
		*/
		std::array<std::array<ord2_t, O2>, O2> counts = {{0}};

		for (ord2_t block = 0; block < O2; block++) {
			ord4_t block_offset = O1 * ((O2 * (block / O1)) + (block % O1));
			for (ord1_t atom = 0; atom < O1; atom++) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{
							// horizontal atom
							const ord4_t offset = block_offset + (O2 * atom);
							const ord2_t a = input_[offset + atom_i], b = input_[offset + atom_j];
							counts[a][b]++; counts[b][a]++;
						}{
							// vertical atom
							const ord4_t offset = block_offset + atom;
							const ord2_t a = input_[offset + (O2 * atom_i)], b = input_[offset + (O2 * atom_j)];
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

					static constexpr std::array<long, O2+1> O2_choose_r = [](){
						std::array<long, O2+1> arr;
						for (int i = 1; i < 1 + O2/2; i++) {
							arr[O2-i] = arr[i] = n_choose_r(O2, i);
						}
						return arr;
					}();
					double p_binomial = static_cast<double>(
						O2_choose_r[count] * std::pow(2, count) * std::pow(O1-1, O2-count)
					) / std::pow(O1+1, O2);

					static constexpr double expected = static_cast<double>(2*O2)/(O1+1);
					canon2orig[label].sort_basis += p_binomial * std::pow(
						static_cast<double>(count) - expected,
					2);
				}
			}
		}
		// Make the lower-valued labels "play favourites":
		std::ranges::sort(canon2orig, std::ranges::greater(), &SortMapEntry::sort_basis);
		// std::cout << "\n"; for (auto e : canon2orig) { std::cout << e.sort_basis << "  "; }

		for (auto& e : input_) {
			e = canon2orig[e].orig;
		}
		// The below only done for debugging purposes.
		/* decltype(counts) counts_sorted;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				counts_sorted[i][j] = counts[canon2orig[i].orig][canon2orig[j].orig];
			}
			std::ranges::sort(counts_sorted[i]);
		}
		counts = counts_sorted; */

		const std::vector<print::print_grid_t> grid_accessors = {
			print::print_grid_t([&input_](std::ostream& os, uint16_t coord) {
				os << ' '; print::val2str(os, O, input_[coord]);
			}),
		};
		print::pretty(std::cout, O, grid_accessors);
	}


	// returns true if lhs is less than rhs
	template<Order O>
	[[gnu::const]] bool cmp_less_atom_slides(
		std::array<typename size<O>::ord2_t, O>& lhs,
		std::array<typename size<O>::ord2_t, O>& rhs
	) {
		for (ord1_t i = 0; i < O1; i++) {
			if      (lhs[i] < rhs[i]) { return true; }
			else if (lhs[i] > rhs[i]) { return false; }
		}
		return false;
	}


	template<Order O>
	void Canonicalizer<O>::movement_() {
		std::array<std::array<std::array<ord2_t, O1>, O1>, O2> row_atom_slides;
		for (ord2_t row = 0; row < O2; row++) {
			for (ord1_t atom = 0; atom < O1; atom++) {
				auto& atom_slide = row_atom_slides[row][atom];
				for (ord1_t cell = 0; cell < O1; cell++) {
					atom_slide[cell] = input_[(O2*row)+(O1*atom)+cell];
				}
				std::ranges::sort(atom_slide, std::ranges::greater());
				for (ord1_t cell = O1-1; cell > 0; cell--) {
					atom_slide[cell-1] += atom_slide[cell];
				}
			}
			std::sort(row_atom_slides[row], cmp_less_atom_slides);
		}
		/* Row and column blocks can be sorted according to mapping each
		block to some reduction of its atoms. Perhaps the product of the
		product of the bits of distance between ones in the mask (including
		the distance to the first one bit)? Or maybe mapping to the index/
		id of the combination (O2 choose O1) (ex. 111000000 would be 0,
		110100000 would be 1).

		Perhaps a similar idea can be used for sorting rows and columns
		withing row/column blocks.
		
		Worth noting that taking the products of the masks probably does
		a similar effect, but definitely requires many more bits. */

		/*
		Another option: For each row, get a view of its atoms. Sort within
		the atoms, and then sort the atoms relative to one another (make a
		cmp_atoms function (that assumes the atom is sorted)).

		reuse the cmp_atoms function inside a cmp_rows function and use that
		to sort the rows within a row block.
		
		use the cmp_rows function to make a cmp_row_blocks function and use
		that to sort the row blocks.

		use the cmp_row_blocks to decide whether the original transposition
		of the grid (now sorted) vs the transposed, sorted grid should be
		used.
		*/
	}


	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept {
		Canonicalizer<O> canon; // TODO
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept; \
		template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}