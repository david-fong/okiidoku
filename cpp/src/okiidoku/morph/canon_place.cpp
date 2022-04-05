#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/traits.hpp>

#include <ranges>
#include <algorithm> // sort
#include <numeric>   // iota
#include <compare>   // weak_ordering
#include <cassert>

namespace okiidoku::morph {

	template<Order O>
	using line_map_t = Transformation<O>::line_map_t;


	template<Order O>
	requires (is_order_compiled(O))
	class CanonPlace final {
		using val_t = traits<O>::o2i_smol_t;
		using mapping_t = Transformation<O>::mapping_t;
		using o1i_t = traits<O>::o1i_t;
		using o2i_t = traits<O>::o2i_t;
		using o4i_t = traits<O>::o4i_t;

	public:
		static constexpr o1i_t O1 = O;
		static constexpr o2i_t O2 = O*O;
		static constexpr o4i_t O3 = O*O*O;
		static constexpr o4i_t O4 = O*O*O*O;

		explicit CanonPlace(const grid_span_t<O> grid): src_grid{grid} {}

	private:
		grid_span_t<O> src_grid;

		struct PolarState final {
			line_map_t<O> to_og;
			std::array<o2i_t, O2> line_tie_links {{0}};
			std::array<o1i_t, O1> chute_tie_links {0};

			explicit constexpr PolarState() noexcept {
				for (o2i_t i {0}; i < O2; ++i) {
					to_og[i/O1][i%O1] = static_cast<mapping_t>(i);
				}
				for (o2i_t i {0}; i < O2; i += O1) {
					line_tie_links[i] = i + O1;
				}
				chute_tie_links[0] = O1;
			}
			bool has_ties() const {
				return std::ranges::any_of(line_tie_links, [](auto link){ return link == 0; })
					|| std::ranges::any_of(chute_tie_links, [](auto link){ return link == 0; });
			}

			void do_a_pass(grid_const_span_t<O>, bool, const PolarState&);
		};

	public:
		static Transformation<O> do_it(const grid_span_t<O> src_grid);
	};


	template<Order O>
	void CanonPlace<O>::PolarState::do_a_pass(
		const grid_const_span_t<O> og_grid,
		const bool is_transpose,
		const PolarState& ortho
	) {
		// I really dislike how complicated this is, but I currently
		// don't know how to simplify it or ease readability.
		grid_arr_flat_t<O> table_arr; {
			const auto t {Transformation<O>{
				Transformation<O>::identity.label_map,
				is_transpose ? ortho.to_og : to_og,
				is_transpose ? to_og : ortho.to_og,
				is_transpose,
			}};
			t.inverted().apply_from_to(og_grid, table_arr);
		}
		GridSpan2D<O> table(table_arr);

		std::array<mapping_t, O2> to_tied;
		std::iota(to_tied.begin(), to_tied.end(), 0);
		// loop over tied line ranges:
		for (o2i_t tie_begin {0}; tie_begin < O2;) {
			const o2i_t tie_end = line_tie_links[tie_begin];
			// TODO note: do not skip ties here
			// loop over the tied line range:
			for (o2i_t rel_i {tie_begin}; rel_i < tie_end; ++rel_i) {
				auto row = table[rel_i];
				// loop over orthogonal partially-resolved line ranges to normalize:
				for (o2i_t t_begin {0}; t_begin < O2;) {
					const o2i_t t_end = ortho.line_tie_links[t_begin];
					std::sort(
						std::next(row.begin(), t_begin),
						std::next(row.begin(), t_end  ),
						std::less{}
					);
					t_begin = t_end;
				}
				// loop over orthogonal partially-resolved chute ranges to normalize:
				for (o1i_t t_begin {0}; t_begin < O1;) {
					const o1i_t t_end = ortho.chute_tie_links[t_begin];
					std::array<o1i_t, O1> resolve;
					std::iota(resolve.begin(), resolve.end(), 0);
					std::ranges::sort(resolve, [&](auto a, auto b){
						return std::ranges::lexicographical_compare(row.subspan(a*O1,O1), row.subspan(b*O1,O1));
					});
					std::array<val_t, O2> copy;
					std::copy(row.begin(), row.end(), copy.begin());
					for (o1i_t i {0}; i < O1; ++i) {
						std::copy(
							std::next(copy.begin(), i*O1), std::next(copy.begin(), (i+1)*O1),
							std::next(row.begin(), i*O1)
						);
					}
					t_begin = t_end;
				}
			}
			std::sort(
				std::next(to_tied.begin(), tie_begin),
				std::next(to_tied.begin(), tie_end  ),
				[&](auto a, auto b){ return std::ranges::lexicographical_compare(table[a], table[b]); } // TODO.try can this be changed to just use array "<" operator?
			);
			tie_begin = tie_end;
		}
		// loop over tied chute ranges:
		for (o1i_t tie_begin {0}; tie_begin < O1;) {
			const o1i_t tie_end = chute_tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			// try to resolve tied chute ranges:
			std::sort(
				std::next(to_tied.begin(), tie_begin),
				std::next(to_tied.begin(), tie_end),
				[&](auto& a, auto& b){ return std::ranges::lexicographical_compare(
					std::views::transform(a, [&](auto i) -> const auto& { return table[i]; }), // TODO can this be something like `table::operator[]`?
					std::views::transform(b, [&](auto i) -> const auto& { return table[i]; })
				); }
			);
			tie_begin = tie_end;
		}

		// update line_tie_links:
		for (o2i_t tie_begin {0}; tie_begin < O2;) {
			const o2i_t tie_end = line_tie_links[tie_begin];
			o2i_t begin {tie_begin};
			for (o2i_t i {static_cast<o2i_t>(begin+1)}; i < tie_end; ++i) {
				if (!std::ranges::equal(table[to_tied[i-1], table[to_tied[i]])) {
					line_tie_links[begin] = i;
					begin = i;
			}	}
			line_tie_links[begin] = tie_end;
			tie_begin = tie_end;
		}
		// update chute_tie_links:
		for (o1i_t tie_begin {0}; tie_begin < O1;) {
			const o1i_t tie_end = chute_tie_links[tie_begin];
			o1i_t begin {tie_begin};
			for (o1i_t a {static_cast<o1i_t>(begin+1)}; a < tie_end; ++a) {
				if (!std::ranges::equal(
					std::views::transform(to_tied.subspan((a-1)*O1, (a  )*O1), [&](auto i) -> const auto& { return table[i]; }),
					std::views::transform(to_tied.subspan((a  )*O1, (a+1)*O1), [&](auto i) -> const auto& { return table[i]; })
				)) {
					chute_tie_links[begin] = a;
					begin = a;
			}	}
			chute_tie_links[begin] = tie_end;
			tie_begin = tie_end;
		}


		{
			// update s.to_og:
			std::array<mapping_t, O2> tied_to_og;
			for (o2i_t i {0}; i < O2; ++i) {
				tied_to_og[i] = to_og[i/O1][i%O1];
			}
			for (o2i_t i {0}; i < O2; ++i) {
				to_og[i/O1][i%O1] = tied_to_og[to_tied[i]];
			}
		}
	}


	template<Order O>
	Transformation<O> CanonPlace<O>::do_it(const grid_span_t<O> src_grid) {
		PolarState row_state {};
		PolarState col_state {};

		while (row_state.has_ties() || col_state.has_ties()) {
			const auto old_row_state {row_state};
			const auto old_col_state {col_state};
			row_state.do_a_pass(src_grid, false, col_state);
			col_state.do_a_pass(src_grid, true, old_row_state);

			if (  old_row_state.line_tie_links  == row_state.line_tie_links
				&& old_row_state.chute_tie_links == row_state.chute_tie_links
				&& old_col_state.line_tie_links  == col_state.line_tie_links
				&& old_col_state.chute_tie_links == col_state.chute_tie_links
			) {
				// TODO.mid stalemate... current design insufficient?
				break;
			}
			// TODO.mid the above could be optimized to stop doing a polar once stable
		}

		Transformation<O> transformation {
			.label_map {Transformation<O>::identity.label_map},
			.row_map {row_state.to_og},
			.col_map {col_state.to_og},
			.transpose {false},
		};
		transformation = transformation.inverted();
		// TODO.high use two iota views mapped one to src_grid and one to transposed view and lexicographical compare. if transposed less, edit transformation and apply a transpose_only transformation to src_grid in place.
		transformation.apply_in_place(src_grid);
		assert(grid_follows_rule<O>(src_grid));
		return transformation;
	}


	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O> canon_place(const grid_span_t<O> grid) {
		return CanonPlace<O>::do_it(grid);
	}


	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		template class CanonPlace<O_>; \
		template Transformation<O_> canon_place<O_>(grid_span_t<O_>);
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL
}