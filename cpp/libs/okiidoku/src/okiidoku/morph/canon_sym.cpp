// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
/** \file
symbol canonicalization. see :/writings/design/morph.md#Symbol-Canonicalization. */
#include <okiidoku/detail/canon_stages.hpp>

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/detail/util.hpp>

// TODO prune
#include <algorithm>  // swap, sort, ranges::next_permutation
#include <functional> // greater
#include <numeric>    // iota
#include <compare>    // is_eq

namespace okiidoku::mono { namespace {

	template<Order O>
	using sym_map_t = typename Transformation<O>::sym_map_t;


	template<Order O> requires(is_order_compiled(O))
	class CanonSym final {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using to_t = typename Transformation<O>::to_t;

		struct SymRel final {};

		/** for a given grid, sym (AKA "label"), and box, what boxcell is the sym in? */
		struct SymBoxToBoxcellMap final : public detail::Gridlike<O,to_t> {
			using B = SymBoxToBoxcellMap::Gridlike;
			using B::operator[];

			/** \pre grid is filled and follows the one rule. */
			explicit constexpr SymBoxToBoxcellMap(const Grid<O>& grid) noexcept {
				OKIIDOKU_ASSERT(grid.is_filled());
				OKIIDOKU_ASSERT(grid.follows_rule());
				for (const auto row : T::O2) {
				for (const auto col : T::O2) {
					const auto sym {*grid[row, col]};
					const auto box {row_col_to_box<O>(row, col)};
					const auto box_cell {row_col_to_box_cell<O>(row, col)};
					operator[](sym, box) = box_cell;
				}}
			}
			[[nodiscard, gnu::pure]] SymRel quantify_sym_relationship(const to_t a, const to_t b) noexcept {
				a.check(); b.check();
				SymRel ret {};
				return ret;
			}
		};

		sym_map_t<O> to_og_; ///< `map[sym_new] -> sym_orig`.
		detail::Ties<T::O2> ties_ {};
		constexpr CanonSym() noexcept {
			std::iota(to_og_.begin(), to_og_.end(), to_t{0u});
		}
		void do_a_pass(const SymBoxToBoxcellMap&) noexcept;

	public:
		[[nodiscard, gnu::pure]] static sym_map_t<O> get_map(const Grid<O>& grid) noexcept;
	};

	template<Order O> requires(is_order_compiled(O))
	void CanonSym<O>::do_a_pass(const SymBoxToBoxcellMap& sym_box_to_boxcell_map) noexcept {
		// for (const auto sym_a : T::O2) {
		// for (const auto sym_b : T::O2) {
		// 	O2BitArr<O> r_atom, col_atom;
		// 	std::array<HouseType, T::O2> pic;
		// 	for (const auto box : T::O2) {
		// 		const auto boxcell_a {sym_box_to_boxcell_map[sym_a, box]};
		// 		const auto boxcell_b {sym_box_to_boxcell_map[sym_b, box]};
		// 	}
		// }}
		for (const auto tie : ties_) {
			// std::sort(
			// 	std::next(tiebreak_map.begin(), tie.begin_),
			// 	std::next(tiebreak_map.begin(), tie.end_),
			// 	[&](auto a, auto b){ return std::lexicographical_compare(
			// 		scratch.row_span_at(a).begin(), scratch.row_span_at(a).end(),
			// 		scratch.row_span_at(b).begin(), scratch.row_span_at(b).end()
			// 	); } // TODO.low why doesn't the ranges version work?
			// );
		}
		// ties_.update([&](auto a, auto b){
		// 	const auto a_row_sp {scratch.row_span_at(tiebreak_map[a])};
		// 	const auto b_row_sp {scratch.row_span_at(tiebreak_map[b])};
		// 	return std::equal(a_row_sp.begin(), a_row_sp.end(), b_row_sp.begin(), b_row_sp.end());
		// });
	}


	template<Order O> requires(is_order_compiled(O))
	sym_map_t<O> CanonSym<O>::get_map(const Grid<O>& grid) noexcept {
		const SymBoxToBoxcellMap sym_box_to_boxcell_map {grid};
		CanonSym s {};
		while (s.ties_.has_unresolved()) {
			auto old_ties {s.ties_};
			s.do_a_pass(sym_box_to_boxcell_map);
			if (s.ties_.none_resolved()) [[unlikely]] {
				// encountered the most canonical grid?
				break;
			}
			if (old_ties == s.ties_) {
				// TODO.mid stalemate... current design insufficient?
				break;
			}
		}
		sym_map_t<O> map OKIIDOKU_DEFER_INIT;
		for (const auto canon_i : T::O2) { map[s.to_og_[canon_i]] = canon_i; } // (invert map)
		return map;
	}
}}
namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O)) [[nodiscard, gnu::pure]]
	sym_map_t<O> canon_sym(const Grid<O>& grid) noexcept {
		return CanonSym<O>::get_map(grid);
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template sym_map_t<(O_)> canon_sym<(O_)>(const Grid<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}