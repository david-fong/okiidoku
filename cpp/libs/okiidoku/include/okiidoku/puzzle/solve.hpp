#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT SolutionWalker final {
		using T = traits<O>;
		using cands_t = HouseMask<O>;
		using cands_grid_t = detail::Gridlike<cands_t>;
		using coord_t = typename T::o4x_smol_t;

		/* Note: A pretty large structure. Hard to get around it, since
		cell_cands_ has non-trivial management logic (deductive solving)
		so backtracking the DFS isn't as simple as unsetting some house-
		mask bits. */
		struct BfDfsStep final {
			cands_grid_t prev_cell_cands;
			coord_t curr_guessed_coord;
		};
		using BfDfsStack = std::stack<BfDfsStep/* , std::vector<BfDfsStep> */>;

		cands_grid_t cell_cands_;

		BfDfsStack bf_dfs_stack_;

		void find_cells_requiring_symbols() noexcept; // AKA naked subsets
		void find_symbols_requiring_cells() noexcept; // AKA midden subsets
		void find_fish() noexcept;

	public:
		explicit SolutionWalker(const) noexcept;
		// TODO.low either 
		struct Iterator final {
			;
		};
	};
}


namespace okiidoku::visitor::puzzle {
}
#endif