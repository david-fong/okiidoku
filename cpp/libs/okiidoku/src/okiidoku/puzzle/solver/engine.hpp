#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE

#include <okiidoku/puzzle/solver/found_queue.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/export.h>

#include <stack>
#include <memory>      // unique_ptr
#include <type_traits> // is_aggregate_v

/**
This class is a primitive for building a primarily-deductive solver.
It uses a cell-major representation. See the design docs for more info.
It exposes a mechanism to use backtracking (guessing)- intended as a
 fallback after applying at least _some_ deductive techniques.

Currently it is private to the library and used to build the public
 FastSolver and VeryDeductiveSolver.

The contracts require dutiful care to follow. Ie. where it sees fit, it
 optimizes for the performance of its intended use-cases at the cost of
 the mental overhead of knowing and following the resulting contracts.

I imagine an average library _user_ would not be interested in such tinkering.
Of course, anyone can clone the repo and do such tinkering within it if they wish.

Examples of various ways this could be used:
- FastSolver: eagerly/immediately consumes found candidate eliminations
- VeryDeductive: "hoards"/accumulates found candidate eliminations until
	no more can possibly be found without consuming any.
- hypothetical "StupidSolver" purely uses the guess mechanism (_very_
   inefficient, but it would still be able to find all possible solutions)
  - note that the engine saves large restore-states for each guess based
     on the assumption that guesses shouldn't be used excessively.
*/
namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class EngineObj;

	struct SolutionsRemain;

	// usage: must be called immediately when a cell's candidate-symbol count
	//  changes to zero.
	// the most-recent guess gets eliminated, which may also change the guessed cell's
	//  candidate-symbol count to become zero, in which case it "recurses".
	// returns that no solutions remain if the guess stack is empty.
	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain detail_engine_unwind_and_rule_out_bad_guesses_(EngineObj<O>&) noexcept;
	// Note: ^I wanted this to be a private `EngineObj` member, but friendship got ugly.
	// TODO hm... but now I don't like that this function is free and visible. maybe try private again.

	// This class exists to make it difficult for me to forget to internally
	// attempt to unwind the guess stack when required, and to enforce the receiver
	// side to check the result in an always-readable way.
	struct [[nodiscard("must stop if no solutions remain")]] SolutionsRemain final {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		friend SolutionsRemain detail_engine_unwind_and_rule_out_bad_guesses_<O_>(EngineObj<O_>&) noexcept;
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
	using CandsGrid = detail::Gridlike<O, HouseMask<O>>;


	template<Order O> requires(is_order_compiled(O))
	class EngineObj final {
		friend class CandElimFind<O>;  // the class wraps implementations that can only see what they need.
		friend class CandElimApply<O>; // no wrapping. implementations need to see `eliminate_candidate_sym(s)_`.
		friend SolutionsRemain detail_engine_unwind_and_rule_out_bad_guesses_<O>(EngineObj<O>&) noexcept;
	public:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using val_t = int_ts::o2xs_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
		using o4i_t = int_ts::o4i_t<O>;

		struct Guess final {
			rmi_t rmi;
			val_t val;
		};

		explicit EngineObj(const Grid<O>& puzzle) noexcept;

		// The user of the engine must respond to `get_next_solution` with `std::nullopt`
		// if this returns `true`.
		//
		// Note: All candidate elimination techniques have a contract that this returns `false`.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		// the candidate elimination queue is processed in the order of insertion.
		[[nodiscard, gnu::pure]]
		bool has_queued_cand_elims() const noexcept { return !found_queues_.is_empty(); }

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
		void push_guess(Guess) noexcept;

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
		void enqueue_cand_elims_for_new_cell_claim_sym_(rmi_t rmi) noexcept;

		// The specified candidate-symbol is allowed to already be removed.
		OKIIDOKU_NO_EXPORT
		SolutionsRemain cell_elim_cand_sym_(rmi_t rmi, val_t cand) noexcept;

		// The specified candidate-symbols are allowed to already be removed.
		OKIIDOKU_NO_EXPORT
		SolutionsRemain cell_elim_cand_syms_(rmi_t rmi, const HouseMask<O>& to_remove) noexcept;

		OKIIDOKU_NO_EXPORT
		SolutionsRemain cell_retain_only_cand_syms_(rmi_t rmi, const HouseMask<O>& to_retain) noexcept;


		// num_puzzles_found_t num_puzzles_found_ {0};

		CandsGrid<O> cells_cands_;

		// (The alternative is to count the number of cells with only one candidate
		// each time this property is queried). This is faster, but requires careful
		// internal bookkeeping to not accidentally put in a wrong state. Another
		// alternative: middle-ground: keep a bit field with each bit saying whether
		// the cell at that rmi is solved (has only one candidate).
		o4i_t num_puzzle_cells_remaining_ {T::O4};


		// TODO.asap consider a design where the queue is not a data member of the EngineObj
		CandElimQueues<O> found_queues_ {};


		struct OKIIDOKU_NO_EXPORT GuessStackFrame final {
			// nothing that was deducted based on a bad guess can be used, and since
			// deduction logic is non-trivial, the simplest way to revert is to save
			// the entire restore-state. For large grids, that's a lot of memory for
			// each `cells_cands`, so since the data is cold, it is allocated on the
			// heap.
			std::unique_ptr<CandsGrid<O>> prev_cells_cands;
			o4i_t num_puzzle_cells_remaining;
			Guess guess;
			GuessStackFrame(
				const CandsGrid<O>& prev_cells_cands_,
				const o4i_t num_puzzle_cells_remaining_,
				const Guess guess_
			) noexcept:
				prev_cells_cands{std::make_unique<CandsGrid<O>>(prev_cells_cands_)},
				num_puzzle_cells_remaining{num_puzzle_cells_remaining_},
				guess{guess_}
			{}
		};
		using guess_stack_t = std::stack<GuessStackFrame/* , std::vector<GuessStackFrame> */>;

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