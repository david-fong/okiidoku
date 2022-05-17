#ifndef HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE

#include <okiidoku/puzzle/cell_major_deductive_solver/cand_elim_desc.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/export.h>

#include <stack>
#include <queue>
#include <optional>
#include <memory>      // unique_ptr
#include <type_traits> // conditional_t

/**
This class provides solving primitives to design a solver.
Currently it is private and used to build public solvers.
There is some merit to publicizing this class. Maybe in the future.
Part of why it's private is because it still feels quite low-level to use.
It's much more like a micro-library than a micro-framework.
I imagine an average library _user_ would not be interested in such tinkering.
Of course, anyone can clone the repo and do such tinkering within it if they wish.

Examples of various ways this could be used:
- solvers that greedily/immediately consume queued commits
- solvers that "hoard"/accumulate queued commits until solving techniques
	cannot find anything more without consuming queued commits.
- can purely use the guess mechanism (_very_ inefficient, but technically valid approach)
  - note that the solver is not designed to be efficient when used in
    a way that makes heavy use of guessing- quite the opposite!
	 See the guess stack comments for more info.

Please very carefully read and adhere to the contracts. */
namespace okiidoku::mono::detail::cell_major_deductive_solver {

	template<Order O> requires(is_order_compiled(O))
	class EngineObj;

	struct SolutionsRemain;

	// usage: must be called immediately when a cell's candidate-symbol count
	//  changes to zero.
	// the bad guess gets eliminated, which may also change the guessed cell's
	//  candidate-symbol count to become zero, in which case there is recursion.
	// returns that no solutions remain if the guess stack is empty.
	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain unwind_and_rule_out_bad_guesses_(EngineObj<O>&) noexcept;
	// Note: ^I wanted this to be a private `EngineObj` member, but then friending
	// to it from `SolutionsRemain` would get _really_ ugly: the friendship would
	// try to implicitly instantiate the `EngineObj` class, and and fail because
	// the member would be private (invisible to `SolutionsRemain`).

	// This class exists to make it difficult for me to forget to internally
	// attempt to unwind the guess stack when required, and to enforce the receiver
	// side to check the result in an always-readable way.
	struct [[nodiscard("must stop if no solutions remain")]] SolutionsRemain final {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		friend SolutionsRemain unwind_and_rule_out_bad_guesses_<O_>(EngineObj<O_>&) noexcept;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O

		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return !solutions_remain_; }
		// Note: to-bool conversion operator not used because it is not always-readable.

		SolutionsRemain() = delete;
		static constexpr SolutionsRemain yes() noexcept { return SolutionsRemain{true}; }
	private:
		explicit constexpr SolutionsRemain(bool val): solutions_remain_{val} {}
		bool solutions_remain_;
	};


	template<Order O> requires(is_order_compiled(O))
	class Techniques;


	template<Order O> requires(is_order_compiled(O))
	class EngineObj final {
		friend class Techniques<O>;
		friend SolutionsRemain unwind_and_rule_out_bad_guesses_<O>(EngineObj<O>&) noexcept;
	public:
		using T = Ints<O>;
		using o2i_t = typename T::o2i_t;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		explicit EngineObj(const Grid<O>& puzzle) noexcept;

		// Becomes true when unsat is detected and no guesses remain to be unwound.
		// The user of the engine must respond to `get_next_solution` with `std::nullopt`
		// if this returns `true`.
		//
		// Note: All candidate elimination techniques have a contract that this returns `false`.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		// the candidate elimination queue is processed in the order of insertion.
		[[nodiscard, gnu::pure]]
		bool has_enqueued_cand_elims() const noexcept { return !commit_effects_queue_.empty(); }

		// contract: `has_enqueued_cand_elims` returns `true`.
		SolutionsRemain process_first_queued_cand_elims() noexcept;

		// convenience wrapper around `process_first_queued_cand_elims`.
		SolutionsRemain process_all_queued_cand_elims() noexcept;


		[[nodiscard, gnu::pure]]
		o4i_t get_num_puzzle_cells_remaining() const noexcept { return num_puzzle_cells_remaining_; }

		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// strong recommendation: call `process_all_queued_cand_elims` before
		//  making guesses. Rationale: why guess while you can still deduct?
		void push_guess(rmi_t rmi, val_t val) noexcept;

		[[nodiscard, gnu::pure]]
		size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };

		// contract: `no_solutions_remain` returns `false`.
		// contract: `get_num_puzzle_cells_remaining` returns zero.
		// returns a filled grid following the one rule and containing all the puzzle's givens.
		[[nodiscard, gnu::pure]]
		std::optional<Grid<O>> build_solution_obj() const noexcept;


	private:
		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: no previous call in context of the current guess stack has been
		//  made with the same value of `rmi`.
		// post-condition: `val` is registered as the only candidate-symbol at `rmi`.
		OKIIDOKU_NO_EXPORT
		void register_new_given_(rmi_t rmi, val_t val) noexcept;

		// contract: must be called immediately when a cell's candidate-symbol count _changes_ to one.
		// contract: (it follows that) no previous call in the context of the current
		//  guess stack has been made with the same value of `rmi`.
		// contract: (it follows that) the cell at `rmi` has exactly one candidate-symbol.
		// post-condition: decrements `num_puzzle_cells_remaining`.
		OKIIDOKU_NO_EXPORT
		void enqueue_cand_elims_for_new_cell_requires_symbol_(rmi_t rmi) noexcept;

		// The specified candidate is allowed to already be removed.
		OKIIDOKU_NO_EXPORT
		SolutionsRemain eliminate_candidate_sym_(rmi_t rmi, val_t cand) noexcept;


		// num_puzzles_found_t num_puzzles_found_ {0};

		using cand_syms_t = HouseMask<O>;
		using CandSymsGrid = detail::Gridlike<O, cand_syms_t>;
		CandSymsGrid cells_cands_;

		// helper to speed up checking if the grid is solved.
		o4i_t num_puzzle_cells_remaining_ {T::O4};

		struct OKIIDOKU_NO_EXPORT CommitRecord final {
			rmi_t rmi;
			val_t val;
			CommitRecord(const rmi_t rmi_, const val_t val_) noexcept: rmi{rmi_}, val{val_} {}
		};
		std::queue<CommitRecord> commit_effects_queue_ {};

		struct OKIIDOKU_NO_EXPORT GuessRecord final {
			// nothing that was deducted based on a bad guess can be used, and since
			// deduction logic is non-trivial, the simplest way to revert is to save
			// the entire restore-state. For large grids, that's a lot of memory for
			// each `GuessRecord`, so since the data is cold, it is allocated on the
			// heap.
			std::unique_ptr<CandSymsGrid> prev_cells_cands;
			CommitRecord committed;
			GuessRecord(
				const CandSymsGrid& prev_cells_cands_,
				const CommitRecord committed_
			) noexcept:
				prev_cells_cands{std::make_unique<CandSymsGrid>(prev_cells_cands_)},
				committed{committed_}
			{}
		};
		using guess_stack_t = std::stack<GuessRecord/* , std::vector<GuessRecord> */>;

		// when the implemented deductive techniques cannot be used, this saves
		// the current-branch identification for the brute-force DFS, and the
		// information to restore when backtracking.
		guess_stack_t guess_stack_ {};

		bool no_solutions_remain_ {false};
	};


	// disable implicit template instantiation.
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class EngineObj<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif