// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/rel_info.hpp>
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/grid.hpp>

#include <algorithm> // sort
#include <numeric>   // iota
#include <compare>   // is_eq

namespace okiidoku::mono { namespace {

	using okiidoku::mono::detail::Rel;

	template<Order O>
	using label_map_t = typename Transformation<O>::label_map_t;


	template<Order O> requires(is_order_compiled(O))
	class CanonLabel final {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using val_t = int_ts::o2is_t<O>;
		using mapping_t = typename Transformation<O>::mapping_t;

		struct State final {
			detail::Gridlike<O, Rel<O>> rel_table;
			label_map_t<O> to_og;
			detail::TieLinks<O, 2> ties {};
			explicit constexpr State(const Grid<O>& grid) noexcept: rel_table{detail::make_rel_table<O>(grid)} {
				std::iota(to_og.begin(), to_og.end(), mapping_t{0});
			}
			[[nodiscard]] bool has_ties() const noexcept { return ties.has_unresolved(); }
		};
		static void do_a_pass_(State& s) noexcept;

	public:
		static label_map_t<O> do_it(Grid<O>& grid) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	void CanonLabel<O>::do_a_pass_(CanonLabel<O>::State& s) noexcept {
		OKIIDOKU_NO_PRE_INIT_AUTOVAR detail::Gridlike<O, Rel<O>> scratch;

		OKIIDOKU_NO_PRE_INIT_AUTOVAR label_map_t<O> to_tied;
		std::iota(to_tied.begin(), to_tied.end(), mapping_t{0});
		for (const auto tie : s.ties) {
			if (tie.size() == 1) [[likely]] { continue; }
			for (const auto rel_i : tie) {
				const auto row_sp {scratch.row_span_at(rel_i)};
				{ const auto src_sp {s.rel_table.row_span_at(rel_i)}; std::copy(src_sp.begin(), src_sp.end(), row_sp.begin()); }
				// normalize tied slice for later sorting:
				for (const auto [t_begin, t_end] : s.ties) {
					std::sort(std::next(row_sp.begin(), t_begin), std::next(row_sp.begin(), t_end));
					// if (t_begin == tie_begin) {
					// } else {
					// 	; // TODO try sorting preserving vertical slices across rows
					// }
				}
			}
			std::sort(
				std::next(to_tied.begin(), tie.begin_),
				std::next(to_tied.begin(), tie.end_),
				[&](auto a, auto b){ return std::lexicographical_compare(
					scratch.row_span_at(a).begin(), scratch.row_span_at(a).end(),
					scratch.row_span_at(b).begin(), scratch.row_span_at(b).end()
				); } // TODO.low why doesn't the ranges version work?
			);
		}
		s.ties.update([&](auto a, auto b){
			const auto a_row_sp {scratch.row_span_at(to_tied[a])};
			const auto b_row_sp {scratch.row_span_at(to_tied[b])};
			return std::equal(a_row_sp.begin(), a_row_sp.end(), b_row_sp.begin(), b_row_sp.end());
		});

		{
			// update s.to_og:
			OKIIDOKU_NO_PRE_INIT_AUTOVAR label_map_t<O> tied_to_og {s.to_og};
			for (o2i_t i {0}; i < T::O2; ++i) {
				s.to_og[i] = tied_to_og[to_tied[i]];
			}
		}
		// update s.rel_table (optimized version of doing get_rel_table again)
		scratch = s.rel_table;
		for (o2i_t i {0}; i < T::O2; ++i) {
		for (o2i_t j {0}; j < T::O2; ++j) {
			s.rel_table.at(i,j) = scratch.at(to_tied[i], to_tied[j]);
		}}
	}


	template<Order O> requires(is_order_compiled(O))
	label_map_t<O> CanonLabel<O>::do_it(Grid<O>& grid) noexcept {
		OKIIDOKU_NO_PRE_INIT_AUTOVAR const label_map_t<O> label_og_to_canon {[&](){
			OKIIDOKU_NO_PRE_INIT_AUTOVAR State s(grid);
			while (s.has_ties()) {
				auto old_ties {s.ties};
				do_a_pass_(s);
				if (s.ties.is_completely_unresolved()) {
					// TODO.high encountered the most canonical grid. :O not sure what to do here.
					std::abort();
					break;
				}
				if (old_ties == s.ties) {
					// TODO.mid stalemate... current design insufficient?
					break;
				}
			}

			OKIIDOKU_NO_PRE_INIT_AUTOVAR label_map_t<O> _;
			for (o2i_t canon_i {0}; canon_i < T::O2; ++canon_i) {
				_[s.to_og[canon_i]] = static_cast<mapping_t>(canon_i);
			}
			return _;
		}()};

		for (o4i_t i {0}; i < Ints<O>::O4; ++i) {
			grid.at_rmi(i) = static_cast<val_t>(label_og_to_canon[grid.at_rmi(i)]);
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule<O>(grid));
		return label_og_to_canon;
	}
}}
namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O))
	label_map_t<O> canon_label(Grid<O>& grid) noexcept {
		return CanonLabel<O>::do_it(grid);
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template label_map_t<O_> canon_label<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}