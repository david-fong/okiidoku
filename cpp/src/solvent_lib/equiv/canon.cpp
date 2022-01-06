#include <solvent_lib/equiv/canon.hpp>
#include <solvent_lib/print.hpp>
// #include <solvent_util/math.hpp>

#include <iostream>
#include <array>
#include <algorithm> // sort, inner_product
// #include <numeric>   // transform_reduce
// #include <execution> // execution::par_unseq
// #include <cmath>     // pow
// #include <compare>   // partial_ordering

namespace solvent::lib::equiv {

	//
	template<Order O>
	class Canonicalizer final {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
	 private:
		using has_mask_t = typename size<O>::O2_mask_least_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		using grid_arr_t = grid_mtx_t<O>;

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		Canonicalizer(const grid_vec_t<O>&);
		grid_vec_t<O> operator()(void);

	 private:
		grid_mtx_t<O> grid_;

		void canonicalize_labelling_(void) noexcept;
		void canonicalize_positioning_(void);
	};


	template<Order O>
	grid_vec_t<O> canonicalize(const grid_vec_t<O>& input) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		Canonicalizer<O> canon(input);
		return canon();
	}

	template<Order O>
	Canonicalizer<O>::Canonicalizer(const grid_vec_t<O>& input):
		grid_(lib::grid_vec2mtx<O>(input))
	{}

	template<Order O>
	grid_vec_t<O> Canonicalizer<O>::operator()(void) {
		canonicalize_labelling_();
		// canonicalize_positioning_();

		// const std::vector<print::print_grid_t> grid_accessors = {
		// 	print::print_grid_t([this](std::ostream& os, uint16_t coord) {
		// 		os << ' '; print::val2str(os, O, grid_[coord/O2][coord%O2]);
		// 	}),
		// };
		// print::pretty(std::cout, O, grid_accessors);

		grid_vec_t<O> ret = grid_mtx2vec<O>(grid_);
		return ret;
	}


	template<Order O>
	void Canonicalizer<O>::canonicalize_labelling_(void) noexcept {
		// TODO
		/* struct SortMapEntry final {
			ord2_t orig; // The original label value
			double dist;
			double p_all;
			[[gnu::pure]] std::partial_ordering operator<=>(const SortMapEntry& that) const {
				// auto cmp = p_all <=> that.p_all;
				// if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				// return dist <=> that.dist;
				return p_all <=> that.p_all;
			}
		};
		// Make the lower-valued labels "play favourites":
		std::sort(canon2orig_label.begin(), canon2orig_label.end());
		// std::cout << "\n"; for (auto e : canon2orig_label) { std::cout << e.joint_prob << "  "; }
		{
			auto p_prev = canon2orig_label[0];
			for (ord2_t i = 1; i < O2; i++) {
				const auto p = canon2orig_label[i];
				if (p.dist == p_prev.dist) [[unlikely]] {
					rel_count_tie_mask_[i-1] = true; rel_count_tie_mask_[i] = true; 
				}
				p_prev = p;
			}
		}
		std::array<ord2_t, O2> label_map = {0};
		for (ord2_t i = 0; i < O2; i++) {
			label_map[canon2orig_label[i].orig] = i;
		}
		for (auto& row : grid_) {
			for (auto& e : row) {
				e = label_map[e];
		}	}
		decltype(rel_count_) canon_counts;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				canon_counts[label_map[i]][label_map[j]] = rel_count_[i][j];
		}	}
		rel_count_ = canon_counts; */
	}


	template<Order O>
	void Canonicalizer<O>::canonicalize_positioning_() {
		// TODO
		/* const GridSortEntry grid_slide = GridSortEntry::build(rel_count_, grid_);
		const GridSortEntry transposed_grid_slide = [this](){
			decltype(grid_) transposed_input;
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					transposed_input[i][j] = grid_[j][i];
				}
			}
			return GridSortEntry::build(rel_count_, transposed_input);
		}();

		decltype(grid_) canon_input = {O2};
		for (ord2_t canon_row = 0; canon_row < O2; canon_row++) {
			const auto& r_chute = grid_slide[canon_row/O1];
			const ord2_t orig_row = (O1*r_chute.orig_chute) + r_chute[canon_row%O1].orig_blkline;
			for (ord2_t canon_col = 0; canon_col < O2; canon_col++) {
				const auto& c_chute = transposed_grid_slide[canon_col/O1];
				const ord2_t orig_col = (O1*c_chute.orig_chute) + c_chute[canon_col%O1].orig_blkline;
				canon_input[canon_row][canon_col] = grid_[orig_row][orig_col];
			}
		}
		if (transposed_grid_slide < grid_slide) {
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					grid_[i][j] = canon_input[j][i];
				}
			}
		} else {
			grid_ = canon_input;
		} */
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template grid_vec_t<O_> canonicalize<O_>(const grid_vec_t<O_>&); \
		template class Canonicalizer<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}