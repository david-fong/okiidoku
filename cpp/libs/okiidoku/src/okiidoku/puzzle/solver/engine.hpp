#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE

#include <okiidoku/puzzle/solver/cand_elim_queues.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/export.h>

#include <stack>
#include <memory>      // unique_ptr
#include <type_traits> // conditional_t

/**
This class provides solving primitives to design a solver.
It uses a cell-major representation. See the design docs for more info.
Currently it is private and used to build public solvers.
The contracts require dutiful care to follow.
I imagine an average library _user_ would not be interested in such tinkering.
Of course, anyone can clone the repo and do such tinkering within it if they wish.

Examples of various ways this could be used:
- solvers that greedily/immediately consume deduced candidate eliminations
- solvers that "hoard"/accumulate deduced candidate eliminations until no more
	can possibly be found without consuming any.
- can purely use the guess mechanism (_very_ inefficient, but technically valid approach)
  - note that the solver is not designed to be efficient when used in
    a way that makes heavy use of guessing- quite the opposite!
	 See the guess stack comments for more info.

Please very carefully read and adhere to the contracts. */
namespace okiidoku::mono::detail::solver {

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
	// Note: ^I wanted this to be a private `EngineObj` member, but friendship got ugly.

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


	template<Order O> requires(is_order_compiled(O)) class CandElimFind;
	template<Order O> requires(is_order_compiled(O)) class CandElimApply;


	template<Order O> requires(is_order_compiled(O))
	class EngineObj final {
		// TODO.asap is there a good way to not require friending?
		//  Rationale: want to avoid showing and _allowing_ engine fields as non-const / showing non-const
		//   member functions to the finder that it doesn't need to be able to see / mutate.
		//  pass specific fields instead of the whole engine?
		//  I'd like to have the engine to do some prelude things like checking `no_solutions_remain` and `get_num_puzzle_cells_remaining`.
		//  The finder basically only needs to see the cells_cands_ field and the cand_elim_queue_ field.
		//  But I wouldn't want a solution that allows an engine _user_ to pass a finder an engine from one solver
		//   with a queue from a _different_ solver.
		//  Solution: make a lightweight class wrapper for the engine _user_. It will be friended, but only to do those prelude things.
		//   It will do the prelude things and then pass the necessary fields of the engine to a main-body finder that is private to the library.
		// TODO.asap think of what to do for the apply class.
		friend class CandElimFind<O>;
		friend class CandElimApply<O>;
		friend SolutionsRemain unwind_and_rule_out_bad_guesses_<O>(EngineObj<O>&) noexcept;
	public:
		using T = Ints<O>;
		using o2i_t = typename T::o2i_t;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		explicit EngineObj(const Grid<O>& puzzle) noexcept;

		// The user of the engine must respond to `get_next_solution` with `std::nullopt`
		// if this returns `true`.
		//
		// Note: All candidate elimination techniques have a contract that this returns `false`.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		// the candidate elimination queue is processed in the order of insertion.
		[[nodiscard, gnu::pure]]
		bool has_queued_cand_elims() const noexcept { return !cand_elim_queues_.empty(); }

		// contract: `has_queued_cand_elims` returns `true`.
		SolutionsRemain process_first_queued_cand_elims() noexcept;

		// convenience wrapper around `process_first_queued_cand_elims`.
		SolutionsRemain process_all_queued_cand_elims() noexcept;


		[[nodiscard, gnu::pure]]
		o4i_t get_num_puzzle_cells_remaining() const noexcept { return num_puzzle_cells_remaining_; }

		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// guideline: only call when `has_queued_cand_elims` returns `false`. There
		//  is _never_ a good reason to make a guess when you have a deduction ready.
		//  If you do otherwise, those queued candidate eliminations will be discarded
		//  if/when a guess is popped. See design docs for fun discussion on why.
		void push_guess(rmi_t rmi, val_t val) noexcept;

		[[nodiscard, gnu::pure]]
		std::size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };

		// contract: `no_solutions_remain` returns `false`.
		// contract: `get_num_puzzle_cells_remaining` returns zero.
		// returns a filled grid following the one rule and containing all the puzzle's givens.
		[[nodiscard, gnu::pure]]
		Grid<O> build_solution_obj() const noexcept;


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

		// (The alternative is to count the number of cells with only one candidate
		// each time this property is queried). This is faster, but requires careful
		// internal bookkeeping to not accidentally put in a wrong state.
		o4i_t num_puzzle_cells_remaining_ {T::O4};

		CandElimQueues<O> cand_elim_queues_ {};

		struct OKIIDOKU_NO_EXPORT GuessRecord final {
			// nothing that was deducted based on a bad guess can be used, and since
			// deduction logic is non-trivial, the simplest way to revert is to save
			// the entire restore-state. For large grids, that's a lot of memory for
			// each `cells_cands`, so since the data is cold, it is allocated on the
			// heap.
			std::unique_ptr<CandSymsGrid> prev_cells_cands;
			rmi_t guess_rmi;
			val_t guess_val;
			GuessRecord(const CandSymsGrid& prev_cells_cands_, rmi_t rmi, val_t val) noexcept:
				prev_cells_cands{std::make_unique<CandSymsGrid>(prev_cells_cands_)},
				guess_rmi{rmi},
				guess_val{val}
			{}
		};
		using guess_stack_t = std::stack<GuessRecord/* , std::vector<GuessRecord> */>;

		// when the implemented deductive techniques cannot be used, this saves
		// the current-branch identification for the brute-force DFS, and the
		// information to restore when backtracking.
		guess_stack_t guess_stack_ {};

		bool no_solutions_remain_ {false};
	};


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class EngineObj<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif