#include <okiidoku/puzzle/solve.hpp>

#include <okiidoku/puzzle/solver/cand_elim_apply.hpp>
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>
#include <okiidoku/puzzle/solver/engine.hpp>

#include <functional> // cref
#include <array> // to_array

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver() noexcept: engine_{std::make_unique<detail::solver::Engine<O>>()} {}


	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	void FastSolver<O>::reinit_with_puzzle(const Grid<O>& puzzle, const std::optional<CandSymToIgnore> cand_sym_to_ignore) noexcept {
		if (!engine_) { return; }
		engine_->reinit_with_puzzle(puzzle);
		if (cand_sym_to_ignore) {
			const auto& ignore {*cand_sym_to_ignore};
			const auto check {engine_->do_elim_remove_sym_(ignore.rmi, ignore.val)};
			(void)check; // `no_solutions_remain` will be handled by `get_next_solution`.
		}
		num_solns_found_ = 0;
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		if (!engine_ || engine_->no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (num_solns_found() > 0) {
			assert(e.get_num_puzcells_remaining() == 0);
			const auto check {e.unwind_one_stack_frame()};
			if (check.did_unwind_root()) { return std::nullopt; }
		}
		// using T = Ints<O>;
		using Find = detail::solver::CandElimFind<O>;
		static constexpr auto find_subsets {[](detail::solver::Engine<O>& e_) noexcept {
			if constexpr (O < 5) { // NOLINT(readability-magic-numbers)
				return detail::solver::UnwindInfo::make_no_unwind();
			} else {
				return Find::subsets(e_, e_.get_guess_stack_depth() == 0 ? 4 : 2);
			}
		}};
		using finder_t = detail::solver::UnwindInfo (*)(detail::solver::Engine<O>&) noexcept;
		const auto finders {std::to_array({
			std::cref(Find::sym_claim_cell),
			// std::cref(Find::locked_cands), // TODO draft implementation not yet tested. please debug and check it behaves as intended.
			std::cref(*static_cast<finder_t>(find_subsets)),
		})};
		while (e.get_num_puzcells_remaining() > 0) [[likely]] {
			{
				using Apply = detail::solver::CandElimApply<O>;
				const auto check {Apply::apply_all_queued(e)};
				if (check.did_unwind_root()) [[unlikely]] { return std::nullopt; }
				if (e.get_num_puzcells_remaining() == 0) [[unlikely]] { break; }
			}
			{
				auto check {detail::solver::UnwindInfo::make_no_unwind()};
				for (const auto& finder : finders) {
					check = finder(e);
					if (check.did_unwind() || e.has_queued_cand_elims()) { break; }
				}
				if (check.did_unwind_root()) [[unlikely]] { return std::nullopt; }
				if (check.did_unwind() || e.has_queued_cand_elims()) {
					continue;
				}
			}
			e.push_guess(Find::good_guess_candidate(e));
		}
		++num_solns_found_;
		return std::optional<Grid<O>>{std::in_place, e.build_solution_obj()};
	}


	template<Order O> requires(is_order_compiled(O))
	VeryDeductiveSolver<O>::VeryDeductiveSolver() noexcept: engine_{std::make_unique<detail::solver::Engine<O>>()} {}


	template<Order O> requires(is_order_compiled(O))
	VeryDeductiveSolver<O>::~VeryDeductiveSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	void VeryDeductiveSolver<O>::reinit_with_puzzle(const Grid<O>& puzzle) noexcept {
		if (!engine_) { return; }
		engine_->reinit_with_puzzle(puzzle);
		num_solns_found_ = 0;
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> VeryDeductiveSolver<O>::get_next_solution() noexcept {
		if (!engine_ || engine_->no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (num_solns_found() > 0) {
			assert(e.get_num_puzcells_remaining() == 0);
			const auto check {e.unwind_one_stack_frame()};
			if (check.did_unwind_root()) { return std::nullopt; }
		}
		while (e.get_num_puzcells_remaining() > 0) [[likely]] {
			using Find = detail::solver::CandElimFind<O>;
			if (Find::sym_claim_cell(e).did_unwind()) [[unlikely]] { continue; }
			// Find::locked_cands(e);
			// TODO call other techniques

			if (e.has_queued_cand_elims()) {
				// TODO but I want to consume the deduction that would lead to the
				//  most/technique-simplest newly-findable candidate eliminations.
				//  How to implement? seems to imply a degree of "BFS".
				using Apply = detail::solver::CandElimApply<O>;
				const auto check {Apply::apply_first_queued(e)};
				if (check.did_unwind_root()) [[unlikely]] { return std::nullopt; }
			} else {
				e.push_guess(Find::good_guess_candidate(e));
			}
		}
		++num_solns_found_;
		return std::optional<Grid<O>>{std::in_place, e.build_solution_obj()};
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class FastSolver<O_>; \
		template class VeryDeductiveSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {
	;
}