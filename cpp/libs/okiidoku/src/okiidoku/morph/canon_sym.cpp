// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

// TODO prune
#include <algorithm>  // swap, sort, ranges::next_permutation
#include <functional> // greater
#include <numeric>    // iota
#include <cstdlib>    // abs
#include <compare>    // is_eq

/** for each pair of symbols, given something like this (each item place is for a box,
showing whether, and in what orientation the two symbols cohabit an atom in that box):
```none
- | |
| | 0
0 0 -
```
find some way to transform it into something placement and letter-agnostic.
each O2*O2 symbol pair (excluding self-pairs) will have such a table, and
when all overlayed together, each position will have O*(O-1)/2 of each of `a`
and `b`, and `O^2 * (O-1)^2` blanks.
- how many boxes do the two symbols cohabit an atom in? sort desc.
- of the atom cohabitations, how many point the same way? sort desc lexicographically of desc sorted per direction
- of the atom cohabitations, how many
- first try to normalize -/|.
	- for each cell,
		- how many cells does it see "pointing to" it?
		- how many cells does it see "orthogonal to" it?
*/

namespace okiidoku::mono { namespace {

	template<Order O>
	using sym_map_t = typename Transformation<O>::sym_map_t;


	template<Order O> requires(is_order_compiled(O))
	class CanonLabel {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using val_t = T::o2is_t;
		using to_t = typename Transformation<O>::to_t;

		struct State {
			/** for a given grid, sym (AKA "label"), and box, what boxcell is the sym in?
			\internal alternatively:
			O^2 * O^2 * [2*log2(O)] foreach sym, foreach box, which boxatoms is it in?
			O^2 * 2*O * O^2 - foreach sym, for each boxatom, in which boxes is it in that boxatom? */
			Grid<O> sym_box_to_boxcell_map;
			sym_map_t<O> to_og; ///< `map[sym_new] -> sym_orig`.
			detail::Ties<O, 2> ties {};
			explicit constexpr State(const Grid<O>& grid) noexcept {
				for (o2i_t row {0}; row < T::O2; ++row) {
				for (o2i_t col {0}; col < T::O2; ++col) {
					const auto box {row_col_to_box<O>(row, col)};
					const auto sym {grid.at(row, col)};
					const auto box_cell = row_col_to_box_cell<O>(row, col);
					sym_box_to_boxcell_map.at(sym, box) = box_cell;
				}}
				std::iota(to_og.begin(), to_og.end(), to_t{0});
			}
			[[nodiscard]] bool has_ties() const noexcept { return ties.has_unresolved(); }
		};
		static void do_a_pass_(State& s) noexcept;

	public:
		static sym_map_t<O> do_it(Grid<O>& grid) noexcept;
	};

	template<Order O> requires(is_order_compiled(O))
	void CanonLabel<O>::do_a_pass_(CanonLabel<O>::State& s) noexcept {
		// for (o2i_t sym_a {0}; sym_a < T::O2; ++sym_a) {
		// for (o2i_t sym_b {0}; sym_b < T::O2; ++sym_b) {
		// 	O2BitArr<O> r_atom, col_atom;
		// 	std::array<HouseType, T::O2> pic;
		// 	for (o2i_t box {0}; box < T::O2; ++box) {
		// 		const auto boxcell_a {sym_box_to_boxcell_map.at(sym_a, box)};
		// 		const auto boxcell_b {sym_box_to_boxcell_map.at(sym_b, box)};
		// 	}
		// }}
		OKIIDOKU_DEFER_INIT sym_map_t<O> tiebreak_map;
		std::iota(tiebreak_map.begin(), tiebreak_map.end(), to_t{0});
		// for (const auto tie : s.ties) {
		// 	if (tie.size() == 1) [[likely]] { continue; }
		// 	for (const auto rel_i : tie) {
		// 	}
		// 	// std::sort(
		// 	// 	std::next(tiebreak_map.begin(), tie.begin_),
		// 	// 	std::next(tiebreak_map.begin(), tie.end_),
		// 	// 	[&](auto a, auto b){ return std::lexicographical_compare(
		// 	// 		scratch.row_span_at(a).begin(), scratch.row_span_at(a).end(),
		// 	// 		scratch.row_span_at(b).begin(), scratch.row_span_at(b).end()
		// 	// 	); } // TODO.low why doesn't the ranges version work?
		// 	// );
		// }
		// s.ties.update([&](auto a, auto b){
		// 	const auto a_row_sp {scratch.row_span_at(tiebreak_map[a])};
		// 	const auto b_row_sp {scratch.row_span_at(tiebreak_map[b])};
		// 	return std::equal(a_row_sp.begin(), a_row_sp.end(), b_row_sp.begin(), b_row_sp.end());
		// });

		{
			// update s.to_og:
			sym_map_t<O> to_og {s.to_og};
			for (o2i_t i {0}; i < T::O2; ++i) {
				s.to_og[i] = to_og[tiebreak_map[i]];
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	sym_map_t<O> CanonLabel<O>::do_it(Grid<O>& grid) noexcept {
		const sym_map_t<O> sym_og_to_canon {[&](){
			OKIIDOKU_DEFER_INIT State s(grid);
			while (s.has_ties()) {
				auto old_ties {s.ties};
				do_a_pass_(s);
				if (s.ties.none_resolved()) {
					// encountered the most canonical grid.
					break;
				}
				if (old_ties == s.ties) {
					// TODO.mid stalemate... current design insufficient?
					break;
				}
			}

			OKIIDOKU_DEFER_INIT sym_map_t<O> map;
			for (o2i_t canon_i {0}; canon_i < T::O2; ++canon_i) {
				map[s.to_og[canon_i]] = static_cast<to_t>(canon_i);
			}
			return map;
		}()};

		for (o4i_t i {0}; i < Ints<O>::O4; ++i) {
			grid.at_rmi(i) = static_cast<val_t>(sym_og_to_canon[grid.at_rmi(i)]);
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule<O>(grid));
		return sym_og_to_canon;
	}
}}
namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O))
	sym_map_t<O> canon_sym(Grid<O>& grid) noexcept {
		return CanonLabel<O>::do_it(grid);
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template sym_map_t<O_> canon_sym<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}