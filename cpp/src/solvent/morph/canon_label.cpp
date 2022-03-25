#include "solvent/morph/rel_info.hpp"
#include "solvent/grid.hpp"

#include <iostream> // TODO.wait remove
#include <algorithm> // sort
#include <array>
#include <numeric>   // iota
#include <compare>   // strong_ordering, is_eq, etc.
#include <cassert>

namespace solvent::morph {

	template<Order O>
	requires (is_order_compiled(O))
	class CanonLabel final {
		using val_t = size<O>::ord2i_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2x_t = size<O>::ord2x_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
	public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;


		static void do_a_pass_(
			grid_arr_t<O, Rel<O>>& rel_table,
			std::array<ord2i_t, O2>& tie_links, 
			std::array<ord2x_t, O2>& canon_to_og
		) noexcept {
			grid_arr_t<O, Rel<O>> scratch;

			std::array<ord2x_t, O2> canon_to_tied;
			std::iota(canon_to_tied.begin(), canon_to_tied.end(), 0);
			for (ord2i_t tie_begin {0}; tie_begin != O2; tie_begin = tie_links[tie_begin]) {
				ord2i_t tie_end = tie_links[tie_begin];
				if (tie_begin + 1 == tie_end) [[likely]] {
					continue; // not a tie.
				}
				if (!((tie_begin == 0) && (tie_end == O2))) {
					std::clog << "\ntied range: [" << int(tie_begin) << ", " << int(tie_end - 1) << "]";
				}
				for (ord2i_t i {tie_begin}; i < tie_end; ++i) {
					auto& row = scratch[i];
					row = rel_table[i];
					for (ord2i_t other_begin {0}; other_begin != O2; other_begin = tie_links[other_begin]) {
						ord2i_t other_end = tie_links[other_begin];
						std::sort(row.begin()+other_begin, row.begin()+other_end, std::less{});
						// if (other_begin == tie_begin) {
						// } else {
						// 	; // TODO try sorting preserving vertical slices across rows
						// }
					}
				}
				std::sort(canon_to_tied.begin()+tie_begin, canon_to_tied.begin()+tie_end, [&](auto a, auto b){ // TODO does this need stable_sort?
					return std::is_lt(scratch[a] <=> scratch[b]);
				});
			}

			{
				// update tie_links:
				ord2i_t begin {0};
				for (ord2i_t canon_i {1}; canon_i < O2; ++canon_i) {
					if (std::is_neq(scratch[canon_to_tied[canon_i - 1]] <=> scratch[canon_to_tied[canon_i]])) {
						tie_links[begin] = canon_i;
						begin = canon_i;
				}	}
				tie_links[begin] = O2;
			}
			// update rel_table (optimized version of doing get_rel_table again)
			scratch = rel_table;
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				rel_table[i][j] = scratch[canon_to_tied[i]][canon_to_tied[j]];
			}}
			{
				// update canon_to_og:
				std::array<ord2x_t, O2> tied_canon_to_og {canon_to_og};
				for (ord2i_t i {0}; i < O2; ++i) {
					canon_to_og[i] = tied_canon_to_og[canon_to_tied[i]];
				}
			}
		}


		//
		static void do_it(const grid_span_t<O> grid) {
			const std::array<ord2x_t, O2> label_og_to_canon = [&](){
				auto rel_table {make_rel_table<O>(grid)};
				std::array<ord2i_t, O2> tie_links {0};
				tie_links[0] = O2;
				std::array<ord2x_t, O2> canon_to_og;
				std::iota(canon_to_og.begin(), canon_to_og.end(), 0);
				while (std::ranges::any_of(tie_links, [](auto link){ return link == 0; })) {
					std::array<ord2i_t, O2> old_tie_links {tie_links};
					do_a_pass_(rel_table, tie_links, canon_to_og);
					if (tie_links[0] == O2) {
						// TODO.high encountered the most canonical grid. :O not sure what to do here.
						assert(false);
						break;
					}
					if (old_tie_links == tie_links) {
						// stalemate... current design insufficient?
						break;
					}
				}

				std::array<ord2x_t, O2> _;
				for (ord2x_t canon_i {0}; canon_i < O2; ++canon_i) {
					_[canon_to_og[canon_i]] = canon_i;
				}
				return _;
			}();

			for (ord4i_t i {0}; i < O4; ++i) {
				grid[i] = static_cast<val_t>(label_og_to_canon[grid[i]]);
			}
			assert(is_sudoku_valid<O>(grid));
		}
	};


	template<Order O>
	void canon_label(const grid_span_t<O> grid) {
		return CanonLabel<O>::do_it(grid);
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template void canon_label<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}