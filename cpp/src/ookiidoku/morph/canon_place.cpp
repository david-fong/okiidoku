#include <ookiidoku/morph/transform.hpp>
#include <ookiidoku/grid.hpp>
#include <ookiidoku/size.hpp>
#include <ookiidoku/morph/rel_info.hpp>

#include <iostream> // TODO.wait remove after done implementing
#include <algorithm> // sort
#include <numeric>   // iota
#include <compare>   // weak_ordering
#include <cassert>

namespace ookiidoku::morph {

	template<Order O>
	using line_map_t = Transformation<O>::line_map_t;


	template<Order O>
	requires (is_order_compiled(O))
	class CanonPlace final {
		using val_t = size<O>::ord2i_least_t;
		using mapping_t = Transformation<O>::mapping_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
		using atom_grid_t = std::array<std::array<std::array<val_t, O>, O>, O*O>;

	public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O3 = O*O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		explicit CanonPlace(const grid_span_t<O> grid): src_grid{grid} {}

	private:
		grid_span_t<O> src_grid;

		struct PolarState final {
			line_map_t<O> to_og;
			std::array<ord2i_t, O2> line_tie_links {{0}};
			std::array<ord1i_t, O1> chute_tie_links {0};

			explicit constexpr PolarState() noexcept {
				for (ord2i_t i {0}; i < O2; ++i) {
					to_og[i/O1][i%O1] = static_cast<mapping_t>(i);
				}
				for (ord2i_t i {0}; i < O2; i += O1) {
					line_tie_links[i] = i + O1;
				}
				chute_tie_links[0] = O1;
			}
			ord2i_t to_og_at(ord2i_t canon_i) const {
				return to_og[canon_i/O1][canon_i%O1];
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
		const grid_const_span_t<O> src_grid,
		const bool is_transpose,
		const PolarState& ortho
	) {
		grid_arr_flat_t<O> table_arr_; {
			const auto t {Transformation<O>{
				Transformation<O>::identity.label_map,
				is_transpose ? ortho.to_og : to_og,
				is_transpose ? to_og : ortho.to_og,
				is_transpose,
			}};
			t.apply_from_to(src_grid, table_arr_);
		}
		const GridSpan2D<O> table {table_arr_};

		line_map_t<O> to_tied;
		for (ord2i_t i {0}; i < O2; ++i) {
			to_tied[i/O1][i%O1] = static_cast<mapping_t>(i);
		}
		// loop over tied line ranges:
		for (ord2i_t tie_begin {0}; tie_begin < O2; tie_begin = line_tie_links[tie_begin]) {
			const ord2i_t tie_end = line_tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			// loop over the tied line range:
			for (ord2i_t rel_i {tie_begin}; rel_i < tie_end; ++rel_i) {
				auto row = table[rel_i];
				// loop over orthogonal partially-resolved line ranges to normalize:
				for (ord2i_t ortho_l_t_begin {0}; ortho_l_t_begin < O2; ortho_l_t_begin = ortho.line_tie_links[ortho_l_t_begin]) {
					const ord2i_t ortho_l_t_end = ortho.line_tie_links[ortho_l_t_begin];
					std::sort(
						std::next(row.begin(), ortho_l_t_begin),
						std::next(row.begin(), ortho_l_t_end),
						std::less{}
					);
				}
				// loop over orthogonal partially-resolved chute ranges to normalize:
				for (ord1i_t ortho_c_t_begin {0}; ortho_c_t_begin < O2; ortho_c_t_begin = ortho.chute_tie_links[ortho_c_t_begin]) {
					const ord1i_t ortho_c_t_end = chute_tie_links[ortho_c_t_begin];
					std::sort(
						std::next(row.begin(), ortho_c_t_begin),
						std::next(row.begin(), ortho_c_t_end),
						[&](auto& a, auto& b){ return std::ranges::lexicographical_compare(
							std::views::transform(a, [&](auto i) { return table[i]; }),
							std::views::transform(b, [&](auto i) { return table[i]; })
						); }
					);
				}
			}
			std::sort(
				std::next(to_tied[tie_begin/O1].begin(), tie_begin%O1),
				std::next(to_tied[tie_begin/O1].begin(), tie_end  %O1),
				[&](auto a, auto b){ return std::ranges::lexicographical_compare(table[a], table[b]); }
			);
		}
		// loop over tied chute ranges:
		for (ord1i_t tie_begin {0}; tie_begin < O2; tie_begin = chute_tie_links[tie_begin]) {
			const ord1i_t tie_end = chute_tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			std::sort(
				std::next(to_tied.begin(), tie_begin),
				std::next(to_tied.begin(), tie_end),
				[&](auto& a, auto& b){ return std::ranges::lexicographical_compare(
					std::views::transform(a, [&](auto i) { return table[i]; }),
					std::views::transform(b, [&](auto i) { return table[i]; })
				); }
			);
		}

		/* TODO.high for() */{
			{
				// update line_tie_links:
				ord2i_t begin {tie_begin};
				for (ord2i_t canon_i {static_cast<ord2i_t>(begin+1)}; canon_i < tie_end; ++canon_i) {
					if (!std::ranges::equal(table[to_tied[canon_i - 1]], table[to_tied[canon_i]])) {
						line_tie_links[begin] = canon_i;
						begin = canon_i;
				}	}
				line_tie_links[begin] = tie_end;
			}
			// TODO.high update chute_tie_links:
		}


		{
			// update s.to_og:
			line_map_t<O> tied_to_og {to_og};
			for (ord2i_t i {0}; i < O2; ++i) {
				to_og[i/O1][i%O1] = tied_to_og[to_tied[i]/O1][to_tied[i]%O1];
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
		}

		auto transformation = Transformation<O>{
			.label_map {Transformation<O>::identity.label_map},
			.row_map {row_state.to_og},
			.col_map {col_state.to_og}
			.transpose {false},
		};
		transformation = transformation.inverted();
		// TODO use two iota views mapped one to src_grid and one to transposed view and lexicographical compare. if transposed less, edit transformation and apply a transpose_only transformation to src_grid in place.
		transformation.apply_in_place(src_grid);
		assert(is_sudoku_valid<O>(src_grid));
		return transformation;
	}


	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O> canon_place(const grid_span_t<O> grid) {
		return CanonPlace<O>::do_it(grid);
	}


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		template class CanonPlace<O_>; \
		template Transformation<O_> canon_place<O_>(grid_span_t<O_>);
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}