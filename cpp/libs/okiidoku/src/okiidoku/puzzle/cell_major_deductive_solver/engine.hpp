#ifndef HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE

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

	enum class [[nodiscard("unsat must be handled")]] CandElimResult : unsigned char {
		ok,
		unsat, // resulted in cell having zero candidate-symbols
	};
	// TODO.asap consider removing the no_match case and having the caller detect it by comparing the
	// size of the candidate elimination queue before and after. note: now that we have a notion of
	// eager-vs-queue option for calling a technique, it reveals that the match_ok variant was really
	// meant to indicate whether the caller should check for queued candidate eliminations. And now
	// that I think of it, it seems useless: it's comparatively like a grain of sand to always check
	// for new queued candidate eliminations after calling a queueing technique.
	enum class [[nodiscard("unsat must be handled")]] TryTechniqueResult : unsigned char {
		no_match, // usage: if received, try a different technique.
		match_ok, // can be used to immediately skip back to trying easier techniques
		match_unsat, // resulted in cell having zero candidate-symbols
	};

	// ADL-capable code-readability wrapper to check a `CandElimResult` for unsat.
	[[nodiscard, gnu::const]] bool hit_unsat(const CandElimResult check) noexcept {
		return check == CandElimResult::unsat;
	}
	[[nodiscard, gnu::const]] bool hit_unsat(const TryTechniqueResult check) noexcept {
		return check == TryTechniqueResult::match_unsat;
	}
	[[nodiscard, gnu::const]] bool match_ok(const TryTechniqueResult check) noexcept {
		return check == TryTechniqueResult::match_ok;
	}


	template<Order O> requires(is_order_compiled(O))
	class LowLevelEngine final {
	public:
		using T = Ints<O>;
		using o2i_t = typename T::o2i_t;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		explicit LowLevelEngine(const Grid<O>& puzzle) noexcept;

		// Becomes true when unsat is detected and no guesses remain to be unwound.
		// The user of the engine must respond to `get_next_solution` with `std::nullopt`
		// if this returns `true`.
		//
		// Note: All candidate elimination techniques have a contract that this returns `false`.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		// the candidate elimination queue is processed in the order of insertion.
		[[nodiscard, gnu::pure]]
		bool has_enqueued_commit_effects() const noexcept { return !commit_effects_queue_.empty(); }

		// contract: `has_enqueued_commit_effects` returns `true`.
		CandElimResult process_one_queued_commit_effects() noexcept;

		// convenience wrapper around `process_one_queued_commit_effects`.
		CandElimResult process_all_queued_commit_effects() noexcept;


		[[nodiscard, gnu::pure]]
		o4i_t get_num_puzzle_cells_remaining() const noexcept { return num_puzzle_cells_remaining_; }

		// contract: `no_solutions_remain` returns `false`.
		// contract: `get_num_puzzle_cells_remaining` returns zero.
		// returns a filled grid following the one rule and containing all the puzzle's givens.
		[[nodiscard, gnu::pure]]
		std::optional<Grid<O>> build_solution_obj() const noexcept;

		// Internal (somewhat obvious) contract:
		// _techniques_ must never incorrectly progress in solving a proper puzzle.r running the same technique again right away...

		// common contracts and invariants for techniques:
		// contract: all techniques require that `no_solutions_remain` returns `false`.
		// behaviour: all techniques immediately return `TryTechniqueResult::no_match`
		//  if `get_num_puzzle_cells_remaining` returns zero.

		// TODO.high probably best that these return right away after solving just one? (gives most control to caller)
		//  but then would that possibly lead to duplicate effort if caller wants to run the same technique again right away?
		//  maybe make two members? one to return after first, one to find as many as possible?
		//  Then again, I don't see the FastSolver or ReasoningSolve
		//  how about this: for techniques that are not (relatively) trivial to perform a full scan of the grid,
		//   provide a parameter enum `SearchEffort { find_first, find_all, };`
		TryTechniqueResult try_technique_symbol_requires_cell() noexcept; // symbol can't go anywhere else in a house

		TryTechniqueResult try_technique_locked_candidates() noexcept;

		static constexpr unsigned technique_subsets_max_subset_size {};

		// AKA "naked subsets"
		// contract: `subset_size` is in the range [2, ((O2+1)//2)].
		TryTechniqueResult try_technique_cells_requiring_symbols(/* val_t subset_size */) noexcept;

		// AKA "hidden subsets"
		// contract: `subset_size` is in the range [2, ((O2+1)//2)].
		TryTechniqueResult try_technique_symbols_requiring_cells(/* val_t subset_size */) noexcept;

		TryTechniqueResult try_technique_fish() noexcept;

		// contract: `val` is currently a candidate-symbol at the cell at `rmi`.
		// strong recommendation: call `process_all_queued_commit_effects` before making guesses.
		// TODO.mid if we add another queue for other candidate eliminations, recommend having processed them here too.
		void push_guess(rmi_t rmi, val_t val) noexcept;

		[[nodiscard, gnu::pure]]
		size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };


	private:
		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: no previous call has been made with the same value of `rmi`.
		// post-condition: `val` is registered as the only candidate-symbol at `rmi`.
		OKIIDOKU_NO_EXPORT
		void register_new_given_(rmi_t rmi, val_t val) noexcept;

		// contract: must be called immediately when a cell's candidate-symbol count _changes_ to one.
		// contract: (it follows that) no previous call has been made with the same value of `rmi`.
		// contract: (it follows that) the cell at `rmi` has exactly one candidate-symbol.
		// post-condition: decrements `num_puzzle_cells_remaining`.
		OKIIDOKU_NO_EXPORT
		void enqueue_commit_effects_for_new_cell_requires_symbol_(rmi_t rmi) noexcept;

		// The specified candidate is allowed to already be removed.
		OKIIDOKU_NO_EXPORT
		CandElimResult eliminate_candidate_sym_(rmi_t rmi, val_t cand) noexcept;

		// TODO.asap call automaticaly when necessary.
		// usage: call this when unsat is encountered.
		// unwinds a guess, and then repeats if ruling out the guess eliminates
		//  the last candidate-symbol of the guessed-at cell. If the stack needs
		//  to unwind and has nothing more to unwind, returns `unsat` and sets
		//  `no_solutions_remain` to `true`. The engine propagates this to the
		//  engine-caller, which should immediately return `std::nullopt`.
		OKIIDOKU_NO_EXPORT
		CandElimResult unwind_and_rule_out_bad_guesses_() noexcept;


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
		struct OKIIDOKU_NO_EXPORT QueuedCandElim final {
			enum class Type : unsigned char { elim_single, retain_multiple, elim_multiple };
			union ValUnion {
				val_t val;
				std::conditional_t<
					(sizeof(cand_syms_t) > sizeof(std::unique_ptr<cand_syms_t>)),
					std::template unique_ptr<cand_syms_t>,
					cand_syms_t
				> mask; // TODO.asap consider wrapping with unique_ptr if mask is large
			};
			// TODO.asap find out best layout to avoid unnecessary padding insertion.
			Type type;
			rmi_t rmi;
			ValUnion val_union;
		}; // TODO.asap use this.
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