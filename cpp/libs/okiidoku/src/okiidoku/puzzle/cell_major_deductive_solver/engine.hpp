#ifndef HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE

#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/export.h>

#include <stack>
#include <queue>
#include <optional>
#include <memory> // unique_ptr

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

	enum class [[nodiscard("unsat must be handled")]] CandElimResult : unsigned char {
		ok,
		unsat, // resulted in cell having zero candidate-symbols
	};
	enum class [[nodiscard("unsat must be handled")]] TryTechniqueResult : unsigned char {
		no_match,
		match_ok,
		match_unsat, // resulted in cell having zero candidate-symbols
	};

	// ADL-capable code-readability wrapper to check a `CandElimResult` for unsat.
	[[nodiscard, gnu::const]] bool hit_unsat(const CandElimResult check) noexcept {
		return check == CandElimResult::unsat;
	}
	// TODO.asap I'm not sure if I like the overload. combined with auto for the check variable,
	//  it may make it easy to forget to check for the remaining match_ok vs. no_match distinction.
	[[nodiscard, gnu::const]] bool hit_unsat(const TryTechniqueResult check) noexcept {
		return check == TryTechniqueResult::match_unsat;
	}


	template<Order O> requires(is_order_compiled(O))
	class LowLevelEngine final {
	public:
		using T = Ints<O>;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		explicit LowLevelEngine(const Grid<O>& puzzle) noexcept;

		// becomes true when unsat is detected and no guesses remain to be unwound.
		//
		// contract: (usage) this should be checked at the beginning of the solving routine
		// to immediately return `std::nullopt` if it returns true.
		//
		// All candidate elimination techniques have a contract that this returns `false`.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		[[nodiscard, gnu::pure]]
		bool has_enqueued_commit_effects() const noexcept { return !commit_effects_queue_.empty(); }

		// contract: `has_enqueued_commit_effects` returns true.
		CandElimResult process_one_queued_commit_effects() noexcept;

		// convenience wrapper around `process_one_queued_commit_effects`.
		CandElimResult process_all_queued_commit_effects() noexcept;


		[[nodiscard, gnu::pure]]
		o4i_t get_num_puzzle_cells_remaining() const noexcept { return num_puzzle_cells_remaining_; }

		// contract: `get_num_puzzle_cells_remaining` returns zero.
		[[nodiscard, gnu::pure]]
		std::optional<Grid<O>> build_solution_obj() const noexcept;

		// Internal (somewhat obvious) contract:
		// _techniques_ must never incorrectly progress in solving a proper puzzle.r running the same technique again right away...

		// common contracts and invariants for techniques:
		// invariant: all techniques immediately return TryTechniqueResult::no_match
		//  if `get_num_puzzle_cells_remaining` returns zero.
		// contract: all techniques require that `no_solutions_remain` returns false.

		// TODO.high probably best that these return right away after solving just one? (gives most control to caller)
		//  but then would that possibly lead to duplicate effort if caller wants to run the same technique again right away?
		//  maybe make two members? one to return after first, one to find as many as possible?
		//  Then again, I don't see the FastSolver or ReasoningSolve
		//  how about this: for techniques that are not (relatively) trivial to perform a full scan of the grid,
		//   provide a parameter enum `SearchEffort { find_first, find_all, };`
		TryTechniqueResult try_technique_symbol_requires_cell() noexcept; // symbol can't go anywhere else in a house

		TryTechniqueResult try_technique_locked_candidates() noexcept;

		// AKA "naked subsets"
		// contract: `subset_size` is in the range [2, ((O2+1)//2)].
		TryTechniqueResult try_technique_cells_requiring_symbols(unsigned subset_size) noexcept;

		// AKA "hidden subsets"
		// contract: `subset_size` is in the range [2, ((O2+1)//2)].
		TryTechniqueResult try_technique_symbols_requiring_cells(unsigned subset_size) noexcept;

		TryTechniqueResult try_technique_fish() noexcept;

		// contract: `val` is currently a candidate-symbol at the cell at `rmi`.
		// strong recommendation: call `process_all_queued_commit_effects` before making guesses.
		// TODO.mid if we add another queue for other candidate eliminations, recommend having processed them here too.
		void push_guess(rmi_t rmi, val_t val) noexcept;

		[[nodiscard, gnu::pure]]
		size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };

		// TODO.asap make this private and call automaticaly, internally when necessary.
		// call this when unsat is encountered.
		// contract: `get_guess_stack_depth` returns non-zero value.
		// returns unsat if unwound to empty guess stack and ruling out the
		//  guess eliminated the last candidate-symbol of that guessed-at cell.
		CandElimResult unwind_and_rule_out_bad_guesses() noexcept;


	private:
		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: no previous call has been made with the same value of `rmi`.
		// post-condition: `val` is registered as the only candidate-symbol at `rmi`.
		OKIIDOKU_NO_EXPORT void register_new_given_(rmi_t rmi, val_t val) noexcept;

		// contract: must be called immediately when a cell's candidate-symbol count _changes_ to one.
		// contract: (it follows that) no previous call has been made with the same value of `rmi`.
		// contract: (it follows that) the cell at `rmi` has exactly one candidate-symbol.
		OKIIDOKU_NO_EXPORT void enqueue_commit_effects_for_new_cell_requires_symbol_(rmi_t rmi) noexcept;

		// If the candidate is already eliminated, returns `ok`.
		OKIIDOKU_NO_EXPORT CandElimResult eliminate_candidate_sym_(rmi_t rmi, val_t cand) noexcept;

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

		/* Note: A pretty large structure. Hard to get around it, since
		cells_cands_ has non-trivial management logic (deductive solving)
		so backtracking the DFS isn't as simple as unsetting some house-
		mask bits. */
		struct OKIIDOKU_NO_EXPORT GuessRecord final {
			CandSymsGrid prev_cells_cands;
			CommitRecord committed;
			GuessRecord(
				const CandSymsGrid& prev_cells_cands_,
				const CommitRecord committed_
			) noexcept:
				prev_cells_cands{prev_cells_cands_},
				committed{committed_}
			{}
		};
		// separately heap allocate each entry since this is large, cold data.
		using guess_stack_t = std::stack<std::unique_ptr<GuessRecord>/* , std::vector<GuessRecord> */>;

		// when the implemented deductive techniques cannot be used, this saves
		// the current-branch identification for the brute-force DFS, and the
		// information to restore when backtracking.
		guess_stack_t guess_stack_ {};

		bool no_solutions_remain_ {false};
	};


	// disable implicit template instantiation.
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class LowLevelEngine<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif