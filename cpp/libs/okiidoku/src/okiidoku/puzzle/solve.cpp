#include <okiidoku/puzzle/solve.hpp>

#include <stack>
#include <bitset>

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_NO_EXPORT FastSolver<O>::Impl final {
	public:
		explicit Impl(const Grid<O>& puzzle) noexcept;
		std::optional<Grid<O>> get_next_solution() noexcept;

	private:
		using T = Ints<O>;
		using cand_syms_t = HouseMask<O>;
		using cand_syms_grid_t = detail::Gridlike<O, cand_syms_t>;
		using val_t = typename T::o2x_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;
		using hit_unsat_t = bool;

		/* Note: A pretty large structure. Hard to get around it, since
		cells_cands_ has non-trivial management logic (deductive solving)
		so backtracking the DFS isn't as simple as unsetting some house-
		mask bits. */
		struct GuessStep final {
			cand_syms_grid_t prev_cells_cands;
			rmi_t curr_guessed_rmi;
			GuessStep(
				const cand_syms_grid_t& prev_cells_cands_,
				const rmi_t curr_guessed_rmi_
			) noexcept: prev_cells_cands{prev_cells_cands_}, curr_guessed_rmi{curr_guessed_rmi_} {}
		};
		using guess_stack_t = std::stack<GuessStep/* , std::vector<GuessStep> */>;


		std::bitset<T::O4> solved_cells_ {};
		cand_syms_grid_t cells_cands_;

		// helper to speed up checking if the grid is solved.
		o4i_t num_puzzle_cells_remaining_ {T::O4};

		// when the implemented deductive techniques cannot be used, this saves
		// the current-branch identification for the brute-force DFS, and the
		// information to restore when backtracking.
		guess_stack_t guess_stack_ {};
		bool no_solutions_remaining_ {false};


		[[nodiscard]] hit_unsat_t remove_cell_candidate_(rmi_t rmi, val_t cand) noexcept;
		[[nodiscard]] hit_unsat_t commit_cell_val_(rmi_t rmi, val_t val) noexcept;

		// solve_cell_requires_symbol_ is trivial with the current representation.
		// [[nodiscard]] hit_unsat_t solve_cell_requires_symbol_() noexcept; // cell can't have anything else
		[[nodiscard]] hit_unsat_t solve_symbol_requires_cell_() noexcept; // symbol can't go anywhere else in a house

		[[nodiscard]] hit_unsat_t solve_cells_requiring_symbols_() noexcept; // AKA "naked subsets"
		[[nodiscard]] hit_unsat_t solve_symbols_requiring_cells_() noexcept; // AKA "hidden subsets"
		[[nodiscard]] hit_unsat_t solve_fish() noexcept;

		[[nodiscard]] hit_unsat_t push_guess_(rmi_t rmi) noexcept;

		/* if the guess stack is not empty, unwinds one entry.
		since the guess stack is only used when the implemented deductive techniques
		are insufficient to do any further candidate eliminations, the caller can
		immediately attempt another guess instead of trying
		*/
		void handle_unsat_() noexcept; // either a bad guess, or the puzzle is unsolvable.
	};


	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		impl_{std::make_unique<FastSolver<O>::Impl>(puzzle)} {}


	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		return impl_->get_next_solution();
	}


	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::Impl::Impl(const Grid<O>& puzzle) noexcept {
		cells_cands_.get_underlying_array().fill(house_mask_ones<O>);
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& given {puzzle.at_rmi(rmi)};
			if (given == T::O2) { continue; }
			const auto hit_unsat {commit_cell_val_(static_cast<rmi_t>(rmi), static_cast<val_t>(given))};
			if (hit_unsat) {
				return;
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::Impl::get_next_solution() noexcept {
		if (no_solutions_remaining_) {
			return std::nullopt;
		}
		// while (num_puzzle_cells_remaining_ != 0) {
			// for (const auto& technique : techniques) {
			// 	const auto old_num_puzzle_cells_remaining {num_puzzle_cells_remaining_};
			// 	technique();
			// 	if ()
			// }
		// }
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
	typename FastSolver<O>::Impl::hit_unsat_t
	FastSolver<O>::Impl::remove_cell_candidate_(
		const FastSolver<O>::Impl::rmi_t rmi,
		const FastSolver<O>::Impl::val_t cand_to_remove
	) noexcept {
		// TODO.asap where should the optimizing check for whether the cell is already solved go?
		//  or should it be done by the caller?
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		if (!cell_cands.test(cand_to_remove)) {
			// this candidate is already ruled out.
		}
		cell_cands.unset(cand_to_remove);
		// TODO.low(?) the current design can easily detect when a cell has no candidate symbols,
		// but it will be "late" to detect when a symbol cannot go anywhere in a house. "late" as
		// in only detecting it as it later manifests as a cell having no candidate symbols. Would
		// it be better to add such a check, which, with the current representation, would have to
		// walk all cells of the three shared houses? Or to allow this to be detected "late"?
		if (cell_cands.count() == 0) [[unlikely]] {
			return hit_unsat_t{true};
		}
		if (cell_cands.count() == 1) [[unlikely]] {
			return commit_cell_val_(rmi, cell_cands.count_lower_zeros_assuming_non_empty_mask());
			// TODO.asap instead of deepening the program stack, enqueue to a commit queue.
		}
		return hit_unsat_t{false};
	}


	template<Order O> requires(is_order_compiled(O))
	typename FastSolver<O>::Impl::hit_unsat_t
	FastSolver<O>::Impl::commit_cell_val_(
		const FastSolver<O>::Impl::rmi_t rmi,
		const FastSolver<O>::Impl::val_t val
	) noexcept {
		assert(num_puzzle_cells_remaining_ > 0);
		assert(!solved_cells_.test(rmi));
		assert(val < T::O2);
		assert(cells_cands_.at_rmi(rmi).test(val));
		solved_cells_.set(rmi);
		cells_cands_.at_rmi(rmi) = cand_syms_t{};
		cells_cands_.at_rmi(rmi).set(val);
		// TODO.asap do remove_cell_candidates_ for all same-house cells
		while (false) {
			// if (neighbour_rmi == rmi) [[unlikely]] { continue; }
			// hit_unsat_t hit_unsat {remove_cell_candidate_(neighbour_rmi, val)};
			// if (hit_unsat) [[unlikely]] {
			// 	return hit_unsat_t{true};
			// }
		}
		--num_puzzle_cells_remaining_;
		return hit_unsat_t{false};
	}


	template<Order O> requires(is_order_compiled(O))
	typename FastSolver<O>::Impl::hit_unsat_t
	FastSolver<O>::Impl::push_guess_(
		const FastSolver<O>::Impl::rmi_t rmi
	) noexcept {
		guess_stack_.emplace(cells_cands_, rmi);
		auto& guess_step {guess_stack_.top()};
		guess_step.prev_cells_cands = cells_cands_;
		guess_step.curr_guessed_rmi = rmi;

		// TODO.asap currently just chooses the lowest-valued candidate symbol.
		//  should we extract to a method in case we later want more sophisticated logic?
		return commit_cell_val_(rmi, cells_cands_.at_rmi(rmi).count_lower_zeros_assuming_non_empty_mask());
	}


	template<Order O> requires(is_order_compiled(O))
	void FastSolver<O>::Impl::handle_unsat_() noexcept {
		if (guess_stack_.empty()) {
			; // uhoh precondition this away? if so, add assert here.
			no_solutions_remaining_ = true;
		}
		auto& step {guess_stack_.top()};
		const auto prev_guessed_rmi {step.curr_guessed_rmi};
		// before restoring old `cells_cands_`,  get the value that was guessed:
		assert(cells_cands_.at_rmi(prev_guessed_rmi).count() == 1);
		const auto prev_guessed_val {cells_cands_.at_rmi(prev_guessed_rmi).count_lower_zeros_assuming_non_empty_mask()};
		cells_cands_ = std::move(step.prev_cells_cands);
		// after restoring old `cells_cands_`, rule out the failed guess value
		// to prevent revisiting that path:
		const auto hit_unset {remove_cell_candidate_(prev_guessed_rmi, prev_guessed_val)};

		guess_stack_.pop();
		if (hit_unset) [[unlikely]] {
			handle_unsat_();
		}
	}


	// Note: no need to explicitly instantiate Impl
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class FastSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::puzzle {
	;
}