#include "solvent/morph/transform.hpp"
#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent/morph/rel_info.hpp"

#include <iostream> // TODO.wait remove after done implementing
#include <algorithm> // sort
#include <numeric>   // iota
#include <compare>   // weak_ordering
#include <cassert>

namespace solvent::morph {

	template<Order O>
	using line_map_t = Transformation<O>::line_map_t;


	template<Order O>
	requires (is_order_compiled(O))
	class CanonPlace final {
		using val_t = size<O>::ord2i_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
		template <class T = size<O>::ord1x_t> using arr_t = std::array<T, O>;

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
			arr_t<arr_t<ord2i_t>> line_tie_links {{0}};
			arr_t<> chute_tie_links {0};

			explicit PolarState() noexcept {
				for (auto& map : to_og) {
					std::iota(map.begin(), map.end(), 0);
				}
				chute_tie_links[0] = O1;
				for (auto& tie_links : line_tie_links) {
					tie_links[0] = O1;
				}
			}
			ord2i_t to_og_at(ord2i_t canon_i) const {
				return to_og[canon_i/O1][canon_i%O1];
			}
			bool has_ties() const {
				return std::ranges::any_of(line_tie_links, [](const auto& chute){
					return std::ranges::any_of(chute, [](auto link){ return link == 0; });
				})
				|| std::ranges::any_of(chute_tie_links, [](auto link){ return link == 0; });
			}

			void do_a_pass(const grid_arr_t<O>& src_grid, bool is_transpose, const PolarState& ortho);
		};
		static void do_a_pass_();

	public:
		static Transformation<O> do_it(const grid_span_t<O> src_grid);
	};


	template<Order O>
	void CanonPlace<O>::PolarState::do_a_pass(const grid_arr_t<O>& src_grid, const bool is_transpose, const PolarState& ortho) {
		arr_t<arr_t<ord2i_t>> to_tied;
		// loop over tied line ranges:
		for (ord1i_t i {}) {
			;
		}
		// loop over tied chute ranges:
	}


	template<Order O>
	Transformation<O> CanonPlace<O>::do_it(const grid_span_t<O> src_grid) {
		PolarState h_state {};
		PolarState v_state {};
		while (h_state.has_ties() || v_state.has_ties()) {
			auto old_h_state {h_state};
			auto old_v_to_og {v_state.to_og};
			
		}

		std::array<val_t, O4> canon_grid {O2};
		for (ord2i_t canon_row {0}; canon_row < O2; ++canon_row) {
			const auto& r_chute = h_grid_slide[canon_row/O1];
			const ord2i_t orig_row = static_cast<ord2i_t>((O1*r_chute.orig_chute) + r_chute[canon_row%O1].orig_blkline);
			for (ord2i_t canon_col {0}; canon_col < O2; ++canon_col) {
				const auto& c_chute = v_grid_slide[canon_col/O1];
				const ord2i_t orig_col = static_cast<ord2i_t>((O1*c_chute.orig_chute) + c_chute[canon_col%O1].orig_blkline);
				canon_grid[O2*canon_row+canon_col] = src_grid[O2*orig_row+orig_col];
			}
		}
		bool transpose;
		if (std::is_gt(v_grid_slide <=> h_grid_slide)) {
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


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template Transformation<O_> canon_place<O_>(grid_span_t<O_>); \
		template class CanonPlace<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}