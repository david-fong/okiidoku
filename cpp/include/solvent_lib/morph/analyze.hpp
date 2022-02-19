#ifndef HPP_SOLVENT_LIB__MORPH__ANALYZE
#define HPP_SOLVENT_LIB__MORPH__ANALYZE

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <array>

namespace solvent::lib::morph {

	template<Order O>
	struct GridStats {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
		using has_mask_t = typename size<O>::O2_mask_least_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

		/**
		A reduction of an approximate measure of positional density
		for breaking `count` ties.
		
		Create an array with entries for each block. If a block has this
		rel, its entry counts same-chute blocks which also have it
		(length: O*2, and entries in [0, 2*O-1)). Then create an array
		where entry i counts entries of A which equal i+1 (length:
		2*O-1, entries in [0, O^2)). Alternatively, if reducing for blocks
		that do _not_ have this rel, count entries of A which equal i.
		Finally, map each entry E at i to E*((O^2)^i) and take the sum.

		Lossless reduction should increase performance (fewer bits to
		compare). The number of bits required for the reduction value
		is log2((O^2)^((2*O-1)+1)-1) rounded up.
		*/
		using density_t =
			std::conditional_t<(O <= 2), std::uint_fast8_t,
			std::conditional_t<(O <= 4), std::uint_fast32_t,
			std::conditional_t<(O <= 6), std::uint_fast64_t,
			std::conditional_t<(O <= 9), unsigned long long,
			void
		>>>>;

		struct RelStats final {
			has_mask_t blocks_h, blocks_v; // one for each dimension of grid. we are 2D.
			ord2_t count; // `std::popcount(blocks_h | blocks_v)`
			density_t density_0;
			density_t density_1;
			ord1_t area; // this can help break density ties.
		};

		grid_mtx_t<O> grid_;

		/*
		Coordinate (A,B) contains the number of atoms (or equivalently,
		blocks) in the grid where the label values A and B coexist.
		Choosing labels A and B and swapping the row A with row B and
		col A with col B corresponds to swapping those label values.

		- The diagonal is all zeroes (cohabitation with self is not counted).
		- Each row (or column) sums to `O2 * (2*(O1-1))`.
		- The maximum possible value at any coordinate is O2.
		- The maximum number of times a value will occur in a row/col is
			O2-2 (I think) based on (2 * O2 * (O1-1)) mod (O2-2).
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
		grid_mtx_t<O, RelStats> rel_count_;

		struct LabelStats {
			double prob;
			std::array<density_t, (2*O)-1> sorted_densities;
		};
		std::array<LabelStats, O*O> labels;
		// blocks;
	};

	/** */
	template<Order O> [[nodiscard]]
	[[nodiscard, gnu::const]] GridStats<O> analyze(const grid_vec_t<O>& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template struct GridStats<O_>; \
		extern template GridStats<O_> analyze<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif