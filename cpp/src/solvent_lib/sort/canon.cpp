#include <solvent_lib/sort/canon.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/prob.hpp>

#include <iostream>
#include <algorithm> // ranges::sort, ranges::greater
#include <array>
#include <cmath>     // pow

namespace solvent::lib::canon {

	template<Order O>
	Canonicalizer<O>::Canonicalizer(std::vector<ord2_t> const& input) {
		for (ord2_t row = 0; row < O2; row++) {
			for (ord2_t col = 0; col < O2; col++) {
				input_[row][col] = input[(row*O2)+col];
			}
		}
	}


	template<Order O>
	std::vector<typename size<O>::ord2_t> Canonicalizer<O>::operator()(void) {
		relabel_();
		movement_();
		std::vector<ord2_t> ans(O4);
		for (ord2_t row = 0; row < O2; row++) {
			for (ord2_t col = 0; col < O2; col++) {
				ans[(row*O2)+col] = input_[row][col];
			}
		}
		return ans;
	}


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

		for (ord2_t line = 0; line < O2; line++) {
			for (ord2_t atom = 0; atom < O2; atom += O1) {
				// Go through all unique pairs in the atom:
				for (ord1_t cell_i = 0; cell_i < O1 - 1; cell_i++) {
					for (ord1_t cell_j = cell_i + 1; cell_j < O1; cell_j++) {
						{ // boxrow
							const ord2_t label_i = input_[line][atom+cell_i], label_j = input_[line][atom+cell_j];
							counts[label_i][label_j]++; counts[label_j][label_i]++;
						}
						{ // boxcol
							const ord2_t label_i = input_[atom+cell_i][line], label_j = input_[atom+cell_j][line];
							counts[label_i][label_j]++; counts[label_j][label_i]++;
						}
					}
				}
			}
		}
		struct SortMapEntry {
			ord2_t orig; // The original label value
			double sort_basis = 0; // Relabelling has no effect on this calculated value.
		};
		std::array<SortMapEntry, O2> canon2orig_label = {};
		// The reduction calculation's result must not depend on the ordering
		// of the counts entries. It should encapsulate a label's degree of
		// preference to being in an atom with some labels more than others.
		// The specific reduction below is the sample standard deviation.
		for (ord2_t label = 0; label < O2; label++) {
			canon2orig_label[label] = SortMapEntry { .orig = label, .sort_basis = 0 };
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
					canon2orig_label[label].sort_basis += p_binomial * std::pow(
						static_cast<double>(count) - expected,
					2);
				}
			}
		}
		// Make the lower-valued labels "play favourites":
		std::ranges::sort(canon2orig_label, std::ranges::greater(), &SortMapEntry::sort_basis);
		// std::cout << "\n"; for (auto e : canon2orig_label) { std::cout << e.sort_basis << "  "; }

		for (auto& row : input_) {
			for (auto& e : row) {
				e = canon2orig_label[e].orig;
			}
		}
		// The below only done for debugging purposes.
		/* decltype(counts) counts_sorted;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				counts_sorted[i][j] = counts[canon2orig_label[i].orig][canon2orig_label[j].orig];
			}
			std::ranges::sort(counts_sorted[i]);
		}
		counts = counts_sorted; */

		const std::vector<print::print_grid_t> grid_accessors = {
			print::print_grid_t([this](std::ostream& os, uint16_t coord) {
				os << ' '; print::val2str(os, O, input_[coord/O2][coord%O2]);
			}),
		};
		print::pretty(std::cout, O, grid_accessors);
	}


	template<Order O>
	bool Canonicalizer<O>::cmp_less_atom_slides::operator()(
		atom_slide_t const& lhs, atom_slide_t const& rhs
	) const {
		for (ord1_t i = 0; i < O; i++) {
			if      (lhs[i] < rhs[i]) { return true; }
			else if (lhs[i] > rhs[i]) { return false; }
		}
		return false; // equal
	}


	template<Order O>
	bool Canonicalizer<O>::cmp_less_line_slides::operator()(
		line_slide_t const& lhs, line_slide_t const& rhs
	) const {
		for (ord1_t line_slider = 0; line_slider < O; line_slider++) {
			// TODO
		}
		return false; // equal
	}


	template<Order O>
	void Canonicalizer<O>::movement_() {
		std::array<std::array<std::array<ord4_t, O1>, O1>, O2> row_atom_slides;
		struct line_sort_t { ord1_t orig; };
		for (ord1_t chute = 0; chute < O1; chute++) {
			for (ord1_t chute_row = 0; chute_row < O1; chute_row++) {
				for (ord2_t atom_offset = 0; atom_offset < O2; atom_offset += O1) {
					auto& atom_slide = row_atom_slides[chute_row][atom_offset];
					for (ord1_t cell = 0; cell < O1; cell++) {
						atom_slide[cell] = input_[chute_row][atom_offset+cell];
					}
					std::ranges::sort(atom_slide, std::ranges::greater());
					for (ord1_t cell = O1-1; cell > 0; cell--) {
						atom_slide[cell-1] += atom_slide[cell];
					}
				}
				std::ranges::sort(row_atom_slides[chute_row], cmp_less_atom_slides());
			}
		}
		
	}


	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		(Canonicalizer<O>(input))(); // TODO
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept; \
		template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}