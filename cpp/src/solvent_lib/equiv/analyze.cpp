#include <solvent_lib/equiv/analyze.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/math.hpp>

#include <iostream>
#include <array>
#include <algorithm> // sort, inner_product
#include <numeric>   // transform_reduce
#include <execution> // execution::par_unseq
#include <cmath>     // pow
#include <compare>   // partial_ordering

namespace solvent::lib::equiv {

	template<Order O>
	class GridInnerRelStats final {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
	 private:
		using has_mask_t = typename size<O>::O2_mask_least_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		using grid_arr_t = grid_mtx_t<O>;

		struct RelCount final {
			has_mask_t blocks_h, blocks_v; // one for each dimension of grid. we are 2D.
			ord2_t count;
		};
		struct LineSortEntry final {
			ord1_t orig_blkline;
			double prob_polar;
			static LineSortEntry build(const grid_mtx_t<O, RelCount>& counts, ord1_t orig_blkline, const std::array<ord2_t, O*O>& line);
			[[gnu::pure]] std::partial_ordering operator<=>(const LineSortEntry& that) const;
		};
		struct ChuteSortEntry final {
			ord1_t orig_chute;
			double prob_all;
			double prob_polar;
			std::array<LineSortEntry, O> lines_;
			static ChuteSortEntry build(const grid_mtx_t<O, RelCount>& counts, ord1_t orig_chute, const grid_arr_t& grid);
			[[gnu::const]] const LineSortEntry& operator[](ord1_t i) const { return lines_[i]; }
			[[gnu::const]] std::partial_ordering operator<=>(const ChuteSortEntry& that) const;
		};
		struct GridSortEntry final {
			double prob;
			std::array<ChuteSortEntry, O> chutes_;
			static GridSortEntry build(const grid_mtx_t<O, RelCount>& counts, const grid_arr_t& grid);
			[[gnu::const]] const ChuteSortEntry& operator[](ord1_t i) const { return chutes_[i]; }
			[[gnu::const]] std::partial_ordering operator<=>(const GridSortEntry& that) const;
		};

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		GridInnerRelStats(const grid_vec_t<O>&);
		grid_vec_t<O> operator()(void);

	 private:
		grid_mtx_t<O> grid_;

		/*
		Coordinate (A,B) contains the number of atoms (or equivalently,
		blocks) in the grid where the label values A and B coexist.
		Choosing labels A and B and swapping the row A with row B and
		col A with col B corresponds to swapping those label values.

		- The diagonal is all zeroes (cohabitation with self is not counted).
		- Each row (or column) sums to `O2 * (2*(O1-1))`.
		- The maximum possible value at any coordinate is O2.
		- The expected value at any non-diagonal coordinate is 2*O2/(O1+1).
			Examples by order: 2: 2.67,  3: 4.50,  4: 6.40,  5: 8.33
		
		- It is a Real Symmetric Matrix -> It is a Normal Matrix -> It is
			diagonalizable by an Orthogonal Matrix.
		- It can be interpreted as a Markov Chain: The walk where starting
			at a cell with a certain value, each transition hops to another
			cell in a same atom and then hops to a random cell with the same
			value.
		- It is a Doubly Stochastic Matrix (when divided by O2*2*(O1-1))
			AKA a Symmetric Markov Chain -> Its stationary distribution is
			the uniform distribution -> It converges to uniformity as steps
			increase.
		- See https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm#Applications_for_real_symmetric_matrices
		- It can be interpreted as / converted to a graph. It is weighted,
			undirected, and dense.
		*/
		grid_mtx_t<O, RelCount> rel_count_;
		void analyze_rel_counts_(void) noexcept;

		/**
		The probability of values A and B being in the same atom within a
		block is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of this ocurring k times in a grid is given by a binomial
		distribution B(o^2, 2/(o+1)).
		*/
		static constexpr std::array<double, O2+1> REL_COUNT_ALL_PROB = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * (1<<i) * std::pow(O1-1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
		static constexpr std::array<double, O2+1> REL_COUNT_POLAR_PROB = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * std::pow(O1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
	};


	template<Order O>
	GridStats<O> analyze(const grid_vec_t<O>& input) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		GridInnerRelStats<O> stats(input);
		return stats();
	}

	template<Order O>
	GridInnerRelStats<O>::GridInnerRelStats(const grid_vec_t<O>& input):
		grid_(lib::grid_vec2mtx<O>(input))
	{
	}

	template<Order O>
	grid_vec_t<O> GridInnerRelStats<O>::operator()(void) {
		analyze_rel_counts_();
		grid_vec_t<O> ret = grid_mtx2vec<O>(grid_);
		return ret;
	}


	template<Order O>
	void GridInnerRelStats<O>::analyze_rel_counts_(void) noexcept {
		// Initialize rel_count_:
		for (ord2_t line = 0; line < O2; line++) {
			for (ord2_t atom = 0; atom < O2; atom += O1) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{ // boxrow
							const ord2_t label_i = grid_[line][atom+atom_i], label_j = grid_[line][atom+atom_j];
							const has_mask_t block_mask = rmi2blk<O>(line, atom);
							rel_count_[label_i][label_j].blocks_h |= block_mask;
							rel_count_[label_j][label_i].blocks_h |= block_mask;
						}
						{ // boxcol
							const ord2_t label_i = grid_[atom+atom_i][line], label_j = grid_[atom+atom_j][line];
							const has_mask_t block_mask = rmi2blk<O>(atom, line);
							rel_count_[label_i][label_j].blocks_v |= block_mask;
							rel_count_[label_j][label_i].blocks_v |= block_mask;
						}
		}	}	}	}
/* 		for (auto& row : rel_count_) { for (auto& rel : row) {
			rel.all = rel.polar_h + rel.polar_v;
			rel.polar_max = std::max(rel.polar_h, rel.polar_v);
		}}
		std::array<LabelStats, O2> canon2orig_label = {};

		// Set up Floyd-Warshall:
		grid_mtx_t<O, double> dist_graph;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				dist_graph[i][j] = std::pow(O2, O2 - rel_count_[i][j].all);
		}	}
		for (ord2_t i = 0; i < O2; i++) { dist_graph[i][i] = 0.0; }

		// Do Floyd-Warshall:
		for (ord2_t k = 0; k < O2; k++) {
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					double other = dist_graph[i][k] + dist_graph[k][j];
					double& it = dist_graph[i][j];
					if (it > other) { it = other; }
		}	}	}

		// Sort vertices by Closeness-Centrality (reciprocal of dist):
		for (ord2_t label = 0; label < O2; label++) {
			const auto& paths = dist_graph[label];
			const double dist = std::accumulate(paths.cbegin(), paths.cend(), 0.0);
			const double p_all = std::transform_reduce(
				std::execution::par_unseq, rel_count_[label].cbegin(), rel_count_[label].cend(),
				1.0, std::multiplies<double>(), [](const auto& rel) { return REL_COUNT_ALL_PROB[rel.all]; }
			);
			canon2orig_label[label] = SortMapEntry {
				.orig = label, .dist = dist, .p_all = p_all,
			};
		} */
	}


	template<Order O>
	GridInnerRelStats<O>::LineSortEntry GridInnerRelStats<O>::LineSortEntry::build(
		const grid_mtx_t<O, RelCount>& counts, const ord1_t orig_blkline, const std::array<ord2_t, O*O>& line
	) {
		double prob_polar = 1.0;
		for (ord2_t atom = 0; atom < O2; atom += O1) {
			for (ord1_t i = 0; i < O1-1; i++) {
				for (ord1_t j = i+1; j < O1; j++) {
					prob_polar *= REL_COUNT_ALL_PROB[counts[line[atom+i]][line[atom+j]].count];
		}	}	}
		return LineSortEntry { .orig_blkline = orig_blkline, .prob_polar = prob_polar };
	}
	template<Order O>
	GridInnerRelStats<O>::ChuteSortEntry GridInnerRelStats<O>::ChuteSortEntry::build(
		const grid_mtx_t<O, RelCount>& counts, const ord1_t orig_chute, const grid_arr_t& grid
	) {
		std::array<LineSortEntry, O> lines;
		for (ord1_t i = 0; i < O1; i++) { lines[i] = LineSortEntry::build(counts, i, grid[(O1*orig_chute)+i]); }
		std::sort(lines.begin(), lines.end());
		double prob_polar = 1.0; for (const auto& e : lines) { prob_polar *= e.prob_polar; }
		double prob_all = prob_polar; // TODO
		return ChuteSortEntry { .orig_chute = orig_chute, .prob_all = prob_all, .prob_polar = prob_polar, .lines_ = lines };
	}
	template<Order O>
	GridInnerRelStats<O>::GridSortEntry GridInnerRelStats<O>::GridSortEntry::build(
		const grid_mtx_t<O, RelCount>& counts, const grid_arr_t& grid
	) {
		std::array<ChuteSortEntry, O> chutes;
		for (ord1_t i = 0; i < O1; i++) { chutes[i] = ChuteSortEntry::build(counts, i, grid); }
		std::sort(chutes.begin(), chutes.end());
		double prob = 1.0; for (const auto& e : chutes) { prob *= e.prob_polar; }
		return GridSortEntry { .prob = prob, .chutes_ = chutes };
	}


	template<Order O>
	std::partial_ordering GridInnerRelStats<O>::LineSortEntry::operator<=>(const LineSortEntry& that) const {
		return prob_polar <=> that.prob_polar;
	}
	template<Order O>
	std::partial_ordering GridInnerRelStats<O>::ChuteSortEntry::operator<=>(const ChuteSortEntry& that) const {
		return prob_all <=> that.prob_all;
	}
	template<Order O>
	std::partial_ordering GridInnerRelStats<O>::GridSortEntry::operator<=>(const GridSortEntry& that) const {
		return prob <=> that.prob;
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template GridStats<O_> analyze<O_>(const grid_vec_t<O_>&); \
		template class GridInnerRelStats<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}