#include <okiidoku/morph/rel_info.hpp>
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/traits.hpp>

#include <algorithm> // sort
#include <numeric>   // iota
#include <compare>   // is_eq
#include <cassert>

namespace okiidoku::mono::morph {

	template<Order O>
	using label_map_t = Transformation<O>::label_map_t;


	template<Order O>
	requires (is_order_compiled(O))
	class CanonLabel final {
		using val_t = traits<O>::o2i_smol_t;
		using o1i_t = traits<O>::o1i_t;
		using o2i_t = traits<O>::o2i_t;
		using o4i_t = traits<O>::o4i_t;
	public:
		static constexpr o2i_t O2 = O*O;

	private:
		struct State final {
			GridArr<O, Rel<O>> rel_table;
			label_map_t<O> to_og;
			TieLinks<O, 2> ties {};
			explicit constexpr State(const GridConstSpan<O> grid) noexcept: rel_table{make_rel_table<O>(grid)} {
				std::iota(to_og.begin(), to_og.end(), 0);
			}
			bool has_ties() const { return ties.has_unresolved(); }
		};
		static void do_a_pass_(State& s);

	public:
		static label_map_t<O> do_it(const GridSpan<O> grid);
	};


	template<Order O>
	void CanonLabel<O>::do_a_pass_(CanonLabel<O>::State& s) {
		GridArr<O, Rel<O>> scratch;

		label_map_t<O> to_tied;
		std::iota(to_tied.begin(), to_tied.end(), 0);
		for (const auto tie : s.ties) {
			if (tie.size() == 1) [[likely]] { continue; }
			for (const auto rel_i : tie) {
				auto row = scratch.row_at(rel_i);
				{ auto src {s.rel_table.row_at(rel_i)}; std::copy(src.begin(), src.end(), row.begin()); }
				// normalize tied slice for later sorting:
				for (const auto [t_begin, t_end] : s.ties) {
					std::sort(std::next(row.begin(), t_begin), std::next(row.begin(), t_end));
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
					scratch.row_at(a).begin(), scratch.row_at(a).end(),
					scratch.row_at(b).begin(), scratch.row_at(b).end()
				); } // TODO.low why doesn't the ranges version work?
			);
		}
		s.ties.update([&](auto a, auto b){
			auto a_row {scratch.row_at(to_tied[a])}, b_row {scratch.row_at(to_tied[b])};
			return std::equal(a_row.begin(), a_row.end(), b_row.begin(), b_row.end());
		});

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
			s.rel_table.at(i,j) = scratch.at(to_tied[i], to_tied[j]);
		}}
	}


	template<Order O>
	label_map_t<O> CanonLabel<O>::do_it(const GridSpan<O> grid) {
		const label_map_t<O> label_og_to_canon = [&](){
			State s(grid);
			while (s.has_ties()) {
				auto old_ties {s.ties};
				do_a_pass_(s);
				if (s.ties.is_completely_unresolved()) {
					// TODO.high encountered the most canonical grid. :O not sure what to do here.
					assert(false);
					break;
				}
				if (old_ties == s.ties) {
					// TODO.mid stalemate... current design insufficient?
					break;
				}
			}

			label_map_t<O> _;
			for (o2i_t canon_i {0}; canon_i < O2; ++canon_i) {
				_[s.to_og[canon_i]] = static_cast<Transformation<O>::mapping_t>(canon_i);
			}
			return _;
		}();

		for (o4i_t i {0}; i < traits<O>::O4; ++i) {
			grid[i] = static_cast<val_t>(label_og_to_canon[grid[i]]);
		}
		assert(grid_follows_rule<O>(grid));
		return label_og_to_canon;
	}


	template<Order O>
	requires (is_order_compiled(O))
	label_map_t<O> canon_label(const GridSpan<O> grid) {
		return CanonLabel<O>::do_it(grid);
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template label_map_t<O_> canon_label<O_>(GridSpan<O_>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}