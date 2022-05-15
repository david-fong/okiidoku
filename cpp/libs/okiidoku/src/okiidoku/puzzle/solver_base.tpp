#ifndef TPP_OKIIDOKU__PUZZLE__SOLVER_BASE
#define TPP_OKIIDOKU__PUZZLE__SOLVER_BASE

#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/export.h>

#include <algorithm>
#include <stack>
#include <queue>
#include <memory> // unique_ptr

namespace okiidoku::mono::detail::cell_major_deductive_solver {
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
	- can purely use the guess mechanism (very inefficient, but technically valid approach)

	Please carefully read and adhere to the contracts.
	*/
	template<Order O> requires(is_order_compiled(O))
	class LowLevelEngine final {
	public:
		using T = Ints<O>;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		enum class [[nodiscard("unsat must be handled")]] CandElimResult : unsigned char {
			ok,
			unsat, // resulted in cell having zero candidate-symbols
		};
		enum class [[nodiscard("unsat must be handled")]] TryTechniqueResult : unsigned char {
			no_match,
			match_ok,
			match_unsat, // resulted in cell having zero candidate-symbols
		};

		explicit LowLevelEngine(const Grid<O>& puzzle) noexcept;

		// contract: this should be checked at the beginning of the solving routine
		// to immediately return `std::nullopt` if it returns true.
		[[nodiscard, gnu::pure]]
		bool no_solutions_remain() const noexcept { return no_solutions_remain_; }

		// contract: `val` is a candidate-symbol at `rmi`.
		// contract: no previous call has been made with the same value of `rmi`.
		void commit_and_enqueue_effects(rmi_t rmi, val_t val) noexcept;

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
		// _techniques_ must never incorrectly progress in solving a proper puzzle.

		// try_technique_cell_requires_symbol is trivial with the current representation.
		// CandElimResult try_technique_cell_requires_symbol() noexcept; // cell can't have anything else
		// TODO.high probably best that these return right away after solving just one? (gives most control to caller)
		//  but then would that possibly lead to duplicate effort if caller wants to run the same technique again right away?
		//  maybe make two members? one to return after first, one to find as many as possible?
		//  Then again, I don't see the FastSolver or ReasoningSolver running the same technique again right away...
		TryTechniqueResult try_technique_symbol_requires_cell() noexcept; // symbol can't go anywhere else in a house

		TryTechniqueResult try_technique_locked_candidates() noexcept;

		TryTechniqueResult try_technique_cells_requiring_symbols() noexcept; // AKA "naked subsets"
		TryTechniqueResult try_technique_symbols_requiring_cells() noexcept; // AKA "hidden subsets"
		TryTechniqueResult try_technique_fish() noexcept;

		// contract: `has_enqueued_commit_effects` returns false (ie. all effects are processed).
		// TODO.asap if we add another queue for other candidate eliminations, require having processed them here too.
		// contract: `val` is a candidate-symbol at the cell at `rmi`.
		void push_guess(rmi_t rmi, val_t val) noexcept;

		[[nodiscard, gnu::pure]]
		size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };

		// call this when unsat is encountered.
		// contract: `get_guess_stack_depth` returns non-zero value.
		// returns unsat if unwound to empty guess stack and ruling out the
		//  guess eliminated the last candidate-symbol of that guessed-at cell.
		CandElimResult unwind_and_rule_out_bad_guesses() noexcept;

	private:
		// If the candidate is already eliminated, returns `ok`.
		CandElimResult eliminate_candidate_(rmi_t rmi, val_t cand) noexcept;

		using cand_syms_t = HouseMask<O>;
		using CandSymsGrid = detail::Gridlike<O, cand_syms_t>;
		CandSymsGrid cells_cands_;

		// helper to speed up checking if the grid is solved.
		o4i_t num_puzzle_cells_remaining_ {T::O4};

		struct CommitRecord final {
			rmi_t rmi;
			val_t val;
			CommitRecord(const rmi_t rmi_, const val_t val_) noexcept: rmi{rmi_}, val{val_} {}
		};
		std::queue<CommitRecord> commit_effects_queue_ {};

		/* Note: A pretty large structure. Hard to get around it, since
		cells_cands_ has non-trivial management logic (deductive solving)
		so backtracking the DFS isn't as simple as unsetting some house-
		mask bits. */
		struct GuessStep final {
			CandSymsGrid prev_cells_cands;
			rmi_t curr_guessed_rmi;
			GuessStep(
				const CandSymsGrid& prev_cells_cands_,
				const rmi_t curr_guessed_rmi_
			) noexcept:
				prev_cells_cands{prev_cells_cands_},
				curr_guessed_rmi{curr_guessed_rmi_}
			{}
		};
		// separately heap allocate each entry since this is large, cold data.
		using guess_stack_t = std::stack<std::unique_ptr<GuessStep>/* , std::vector<GuessStep> */>;

		// when the implemented deductive techniques cannot be used, this saves
		// the current-branch identification for the brute-force DFS, and the
		// information to restore when backtracking.
		guess_stack_t guess_stack_ {};

		bool no_solutions_remain_ {false};
	};


	template<Order O> requires(is_order_compiled(O))
	LowLevelEngine<O>::LowLevelEngine(const Grid<O>& puzzle) noexcept {
		cells_cands_.get_underlying_array().fill(house_mask_ones<O>);
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& given {puzzle.at_rmi(rmi)};
			if (given >= T::O2) { continue; }
			commit_and_enqueue_effects(static_cast<rmi_t>(rmi), static_cast<val_t>(given));
		}
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> LowLevelEngine<O>::build_solution_obj() const noexcept {
		if (no_solutions_remain_) [[unlikely]] {
			return std::nullopt;
		}
		assert(num_puzzle_cells_remaining_ == 0);
		assert(( std::all_of(
			cells_cands_.get_underlying_array().cbegin(),
			cells_cands_.get_underlying_array().cend(),
			[](const auto& cands){ return cands.count() == 1; }
		) ));

		// build solution struct and return:
		std::optional<Grid<O>> soln_optional {std::in_place};
		{
			auto& soln {soln_optional.value()};
			for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
				const auto& cell_cands {cells_cands_.at_rmi(rmi)};
				assert(cell_cands.count() == 1);
				soln.at_rmi(rmi) = cell_cands.count_lower_zeros_assuming_non_empty_mask();
			}
		}
		return soln_optional;
	}


	template<Order O> requires(is_order_compiled(O))
	typename LowLevelEngine<O>::CandElimResult
	LowLevelEngine<O>::eliminate_candidate_(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t cand_to_elim
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		if (!cell_cands.test(cand_to_elim)) {
			// candidate was already eliminated.
			return CandElimResult::ok;
		}
		cell_cands.unset(cand_to_elim);
		// TODO.low(?) the current design can easily detect when a cell has no candidate symbols,
		// but it will be "late" to detect when a symbol cannot go anywhere in a house. "late" as
		// in only detecting it as it later manifests as a cell having no candidate symbols. Would
		// it be better to add such a check, which, with the current representation, would have to
		// walk all cells of the three shared houses? Or to allow this to be detected "late"?
		if (cell_cands.count() == 0) [[unlikely]] {
			return CandElimResult::unsat;
		}
		if (cell_cands.count() == 1) [[unlikely]] {
			commit_and_enqueue_effects(rmi, cell_cands.count_lower_zeros_assuming_non_empty_mask());
		}
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	void LowLevelEngine<O>::commit_and_enqueue_effects(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t val
	) noexcept {
		assert(num_puzzle_cells_remaining_ > 0);
		assert(val < T::O2);
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		assert(cell_cands.test(val));
		cell_cands.unset_all();
		cell_cands.set(val);
		assert(cell_cands.test(val));
		commit_effects_queue_.emplace(rmi, val);
		--num_puzzle_cells_remaining_;
	}


	template<Order O> requires(is_order_compiled(O))
	typename LowLevelEngine<O>::CandElimResult
	LowLevelEngine<O>::process_one_queued_commit_effects() noexcept {
		assert(has_enqueued_commit_effects());
		// TODO.asap do eliminate_candidate_ for all same-house cells
		// while (false) {
		// 	if (neighbour_rmi == rmi) [[unlikely]] { continue; }
		// 	CandElimResult hit_unsat {eliminate_candidate_(neighbour_rmi, val)};
		// 	if (hit_unsat) [[unlikely]] {
		// 		return CandElimResult::unsat;
		// 	}
		// }
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	typename LowLevelEngine<O>::CandElimResult
	LowLevelEngine<O>::process_all_queued_commit_effects() noexcept {
		while (has_enqueued_commit_effects()) {
			const auto hit_unsat {process_one_queued_commit_effects()};
			if (hit_unsat == CandElimResult::unsat) {
				return hit_unsat;
			}
		}
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	void LowLevelEngine<O>::push_guess(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t val
	) noexcept {
		assert(!has_enqueued_commit_effects());
		assert(cells_cands_.at_rmi(rmi).test(val));
		guess_stack_.emplace(std::make_unique<GuessStep>(cells_cands_, rmi));

		commit_and_enqueue_effects(rmi, val);
	}


	template<Order O> requires(is_order_compiled(O))
	typename LowLevelEngine<O>::CandElimResult
	LowLevelEngine<O>::unwind_and_rule_out_bad_guesses() noexcept {
		if (guess_stack_.empty()) {
			no_solutions_remain_ = true;
			return CandElimResult::unsat;
		}
		const GuessStep& step {*guess_stack_.top()};
		const auto prev_guessed_rmi {step.curr_guessed_rmi};

		// before restoring old `cells_cands_`,  get the value that was guessed:
		assert(cells_cands_.at_rmi(prev_guessed_rmi).count() == 1);
		const auto prev_guessed_val {cells_cands_.at_rmi(prev_guessed_rmi).count_lower_zeros_assuming_non_empty_mask()};
		cells_cands_ = std::move(step.prev_cells_cands);

		// after restoring old `cells_cands_`, rule out the bad guess value:
		const auto hit_unsat {eliminate_candidate_(prev_guessed_rmi, prev_guessed_val)};

		guess_stack_.pop();
		if (hit_unsat == CandElimResult::unsat) [[unlikely]] {
			return unwind_and_rule_out_bad_guesses();
		}
		return CandElimResult::ok;
	}


	// disable implicit template instantiation.
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class LowLevelEngine<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif