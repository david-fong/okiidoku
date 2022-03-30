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

			explicit PolarState() noexcept {
				for (ord2i_t i {0}; i < O2; ++i) {
					to_og[i/O2][i%O2] = static_cast<mapping_t>(i);
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

			void do_a_pass(const grid_arr_t<O>&, atom_grid_t&, bool, const PolarState&);
		};

	public:
		static Transformation<O> do_it(const grid_span_t<O> src_grid);
	};


	template<Order O>
	void CanonPlace<O>::PolarState::do_a_pass(
		const grid_arr_t<O>& src_grid,
		atom_grid_t& atom_grid,
		const bool is_transpose,
		const PolarState& ortho
	) {
		{
			// TODO update atom grid:
		}
		line_map_t<O> to_tied;
		for (ord2i_t i {0}; i < O2; ++i) {
			to_og[i/O2][i%O2] = i;
		}
		// loop over tied line ranges:
		for (ord2i_t tie_begin {0}; tie_begin < O2; tie_begin = line_tie_links[tie_begin]) {
			ord2i_t tie_end = line_tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			// loop over orthogonally resolved lines:
			for (ord2i_t ortho_tie_begin {0}; ortho_tie_begin < O2; ortho_tie_begin = ortho.line_tie_links[ortho_tie_begin]) {
				ord2i_t ortho_tie_end = ortho.line_tie_links[ortho_tie_begin];
				// TODO.high
			}
		}
		// loop over tied chute ranges:
		for (ord2i_t tie_begin {0}; tie_begin < O2; tie_begin = chute_tie_links[tie_begin]) {
			ord2i_t tie_end = chute_tie_links[tie_begin];
			if ((tie_begin + 1) == tie_end) [[likely]] {
				continue; // not a tie.
			}
			// TODO
		}
	}


	template<Order O>
	Transformation<O> CanonPlace<O>::do_it(const grid_span_t<O> src_grid) {
		PolarState h_state {};
		PolarState v_state {};
		atom_grid_t h_atom_grid;
		atom_grid_t v_atom_grid; // TODO initialize

		while (h_state.has_ties() || v_state.has_ties()) {
			auto old_h_state {h_state};
			auto old_v_to_og {v_state.to_og};
			h_state.do_a_pass(src_grid, h_atom_grid, false, v_state);
			v_state.do_a_pass(src_grid, v_atom_grid, true, old_h_state);
		}

		std::array<val_t, O4> canon_grid {O2};
		for (ord2i_t canon_row {0}; canon_row < O2; ++canon_row) {
			const ord2i_t orig_row = h_state.to_og[canon_row/O1][canon_row%O1];
			for (ord2i_t canon_col {0}; canon_col < O2; ++canon_col) {
				const ord2i_t orig_col = v_state.to_og[canon_row/O1][canon_row%O1];
				canon_grid[(O2*canon_row)+canon_col] = src_grid[(O2*orig_row)+orig_col];
			}
		}
		bool transpose;
		if (std::is_gt(h_atom_grid <=> v_atom_grid)) {
			transpose = true;
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				src_grid[O2*i+j] = canon_grid[O2*j+i];
			}}
			std::swap(h_state, v_state);
		} else {
			transpose = false;
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				src_grid[O2*i+j] = canon_grid[O2*i+j];
			}}
		}
		assert(is_sudoku_valid<O>(src_grid));
		return Transformation<O>{
			.label_map {},
			.transpose {transpose},
			.row_map {h_state.to_og},
			.col_map {v_state.to_og}
		};
	}


	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O> canon_place(const grid_span_t<O> grid) {
		return CanonPlace<O>::do_it(grid);
	}


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		template Transformation<O_> canon_place<O_>(grid_span_t<O_>); \
		template class CanonPlace<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}