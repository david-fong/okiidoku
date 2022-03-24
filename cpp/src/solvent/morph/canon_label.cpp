#include "solvent/morph/rel_info.hpp"
#include "solvent/grid.hpp"

#include <iostream> // TODO.wait remove
#include <algorithm> // sort
#include <vector>
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

		// Info is placement-independent.
		struct NonCanonRelInfo final {
			grid_arr_t<O, Rel<O>> table;
			grid_arr_t<O, Rel<O>> cmp_layers; // sort rows of table and then transpose.

			std::strong_ordering cmp_labels(const ord2x_t a_og_label, const ord2x_t b_og_label) const {
				for (ord2i_t layer {0}; layer < O2; ++layer) {
					const Rel<O>& a = cmp_layers[layer][a_og_label];
					const Rel<O>& b = cmp_layers[layer][b_og_label];
					const auto cmp = a <=> b;
					if (cmp != std::strong_ordering::equivalent) {
						return cmp;
					}
				}
				return std::strong_ordering::equivalent;
			};
		};
		static NonCanonRelInfo make_rel_cmp_layers_(const grid_const_span_t<O> grid_in) noexcept {
			NonCanonRelInfo non_canon;
			grid_arr_t<O, Rel<O>> table_rows_sorted = non_canon.table = get_rel_table<O>(grid_in);
			// sort, transpose (for caching optimization)
			for (auto& row : table_rows_sorted) {
				// put imbalanced things at the front:
				std::ranges::sort(row, std::less{});
			}
			// TODO.try do more benchmarking comparing perf with or without transpose.
			//   some rough benchmarking on O=4 seems to indicate some benefit.
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				non_canon.cmp_layers[i][j] = table_rows_sorted[j][i];
			}}
			return non_canon;
		}


		static void find_and_break_ties_(
			const NonCanonRelInfo& non_canon,
			std::array<ord2x_t, O2>& canon_to_og
		) noexcept {
			struct Range {
				ord2x_t begin;
				ord2i_t end;
				ord2i_t size() const noexcept { return end - begin; };
			};
			std::vector<Range> tied_ranges; // entries are ranges of partially-canonical labels.

			using tieless_mask_t = size<O>::O2_mask_fast_t;
			tieless_mask_t tieless {0}; // bit places correspond to partially-canonical labels.

			for (ord2i_t tie_begin {0}, canon_i {1}; canon_i <= O2; ++canon_i) {
				if (canon_i == O2 || std::is_neq(non_canon.cmp_labels(canon_to_og[tie_begin], canon_to_og[canon_i]))) {
					if (canon_i == tie_begin + 1) [[likely]] {
						tieless |= tieless_mask_t{1} << tie_begin;
					} else {
						tied_ranges.push_back({static_cast<ord2x_t>(tie_begin), canon_i});
					}
					tie_begin = canon_i;
				}
			}
			if (tied_ranges.empty()) [[likely]] {
				return;
			}
			if ((tieless.count() == 0) && (tied_ranges.size() == 1)) {
				// TODO.high encountered the most canonical grid. :O not sure what to do here.
				assert(tied_ranges[0].size() == O2);
				return;
			}
			for (const Range& tied_range : tied_ranges) {
				std::clog << "\ntied range: [" << int(tied_range.begin) << ", " << int(tied_range.end - 1) << "]";
				struct TieBreaker final {
					ord2x_t og_label {};
					std::vector<Rel<O>> rels {}; // indexed by partially-canonical labels.
				};
				std::vector<TieBreaker> tie_breakers(tied_range.size());
				for (ord2i_t breaker_i {0}; breaker_i < tied_range.size(); ++breaker_i) {
					auto& tie_breaker = tie_breakers[breaker_i];
					tie_breaker.og_label = canon_to_og[breaker_i + tied_range.begin];
					tie_breaker.rels.reserve(tieless.count());
					for (ord2i_t canon_i {0}; canon_i < O2; ++canon_i) {
						if ((tieless & (tieless_mask_t{1} << canon_i)).any()) {
							tie_breaker.rels.push_back(non_canon.table[tie_breaker.og_label][canon_to_og[canon_i]]);
					}	}
				}
				std::sort(tie_breakers.begin(), tie_breakers.end(), [](const auto& a, const auto& b){
					return a.rels < b.rels; // std::vector lexicographical comparison
				});
				for (ord2i_t i {tied_range.begin}; i < tied_range.end; ++i) {
					tieless |= tieless_mask_t{1} << i; // mark tie as resolved.
					canon_to_og[i] = tie_breakers[i - tied_range.begin].og_label;
				}
			}
		}


		//
		static void do_it(const grid_span_t<O> grid) {
			const std::array<ord2x_t, O2> label_og_to_canon = [](const NonCanonRelInfo non_canon){
				std::array<ord2x_t, O2> canon_to_og;
				std::iota(canon_to_og.begin(), canon_to_og.end(), 0);
				std::sort(canon_to_og.begin(), canon_to_og.end(), [&](auto a, auto b){
					return std::is_lt(non_canon.cmp_labels(a, b));
				});
				find_and_break_ties_(non_canon, canon_to_og);
				std::array<ord2x_t, O2> _;
				for (ord2x_t canon_i {0}; canon_i < O2; ++canon_i) {
					_[canon_to_og[canon_i]] = canon_i;
				}
				return _;
			}(make_rel_cmp_layers_(grid));

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