#include <ookiidoku/morph/rel_info.hpp>
#include <ookiidoku/morph/transform.hpp>
#include <ookiidoku/grid.hpp>
#include <ookiidoku/traits.hpp>

// #include <iostream>
#include <algorithm> // sort
#include <numeric>   // iota
#include <array>
#include <compare>   // strong_ordering, is_eq, etc.
#include <cassert>

namespace ookiidoku::morph {

	template<Order O>
	using label_map_t = Transformation<O>::label_map_t;


	template<Order O>
	requires (is_order_compiled(O))
	class CanonLabel final {
		using val_t = traits<O>::o2i_smol_t;
		using o1i_t = traits<O>::o1i_t;
		using o2x_t = traits<O>::o2x_t;
		using o2i_t = traits<O>::o2i_t;
		using o4i_t = traits<O>::o4i_t;
	public:
		static constexpr o1i_t O1 = O;
		static constexpr o2i_t O2 = O*O;
		static constexpr o4i_t O4 = O*O*O*O;

	private:
		struct State final {
			grid_arr2d_t<O, Rel<O>> rel_table;
			label_map_t<O> to_og;
			std::array<o2i_t, O2> tie_links {0};
			explicit constexpr State(const grid_const_span_t<O> grid) noexcept: rel_table{make_rel_table<O>(grid)} {
				std::iota(to_og.begin(), to_og.end(), 0);
				tie_links[0] = O2;
			}
			bool has_ties() const {
				return std::ranges::any_of(tie_links, [](auto link){ return link == 0; });
			}
		};
		static void do_a_pass_(State& s) noexcept;

	public:
		static label_map_t<O> do_it(const grid_span_t<O> grid);
	};


	template<Order O>
	void CanonLabel<O>::do_a_pass_(CanonLabel<O>::State& s) noexcept {
		grid_arr2d_t<O, Rel<O>> scratch;

		label_map_t<O> to_tied;
		std::iota(to_tied.begin(), to_tied.end(), 0);
		// loop over tied ranges:
		for (o2i_t tie_begin {0}; tie_begin != O2; tie_begin = s.tie_links[tie_begin]) {
			const o2i_t tie_end = s.tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			// loop over the tied range:
			for (o2i_t rel_i {tie_begin}; rel_i < tie_end; ++rel_i) {
				auto& row = scratch[rel_i];
				row = s.rel_table[rel_i];
				// normalize tied slice for later sorting:
				for (o2i_t t_begin {0}; t_begin != O2; t_begin = s.tie_links[t_begin]) {
					const o2i_t t_end = s.tie_links[t_begin];
					std::sort(
						std::next(row.begin(), t_begin),
						std::next(row.begin(), t_end),
						std::less{}
					);
					// if (t_begin == tie_begin) {
					// } else {
					// 	; // TODO try sorting preserving vertical slices across rows
					// }
				}
			}
			std::sort(
				std::next(to_tied.begin(), tie_begin),
				std::next(to_tied.begin(), tie_end),
				[&](auto a, auto b){ return std::is_lt(scratch[a] <=> scratch[b]); }
			);
			{
				// update s.tie_links:
				o2i_t begin {tie_begin};
				for (o2i_t canon_i {static_cast<o2i_t>(begin+1)}; canon_i < tie_end; ++canon_i) {
					if (std::is_neq(scratch[to_tied[canon_i - 1]] <=> scratch[to_tied[canon_i]])) {
						s.tie_links[begin] = canon_i;
						begin = canon_i;
				}	}
				s.tie_links[begin] = tie_end;
			}
		}

		{
			// update s.to_og:
			label_map_t<O> tied_to_og {s.to_og};
			for (o2i_t i {0}; i < O2; ++i) {
				s.to_og[i] = tied_to_og[to_tied[i]];
			}
		}
		// update s.rel_table (optimized version of doing get_rel_table again)
		scratch = s.rel_table;
		for (o2i_t i {0}; i < O2; ++i) {
		for (o2i_t j {0}; j < O2; ++j) {
			s.rel_table[i][j] = scratch[to_tied[i]][to_tied[j]];
		}}
	}


	template<Order O>
	label_map_t<O> CanonLabel<O>::do_it(const grid_span_t<O> grid) {
		const label_map_t<O> label_og_to_canon = [&](){
			State s(grid);
			while (s.has_ties()) {
				std::array<o2i_t, O2> old_tie_links {s.tie_links};
				do_a_pass_(s);
				if (s.tie_links[0] == O2) {
					// TODO.high encountered the most canonical grid. :O not sure what to do here.
					assert(false);
					break;
				}
				if (old_tie_links == s.tie_links) {
					// TODO.mid stalemate... current design insufficient?
					break;
				}
			}

			std::array<o2x_t, O2> _;
			for (o2x_t canon_i {0}; canon_i < O2; ++canon_i) {
				_[s.to_og[canon_i]] = canon_i;
			}
			return _;
		}();

		for (o4i_t i {0}; i < O4; ++i) {
			grid[i] = static_cast<val_t>(label_og_to_canon[grid[i]]);
		}
		assert(grid_follows_rule<O>(grid));
		return label_og_to_canon;
	}


	template<Order O>
	requires (is_order_compiled(O))
	label_map_t<O> canon_label(const grid_span_t<O> grid) {
		return CanonLabel<O>::do_it(grid);
	}


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		template label_map_t<O_> canon_label<O_>(grid_span_t<O_>);
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}