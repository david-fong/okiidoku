// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER2__ENGINE
#define HPP_OKIIDOKU__PUZZLE__SOLVER2__ENGINE

#include <okiidoku/puzzle2/solver/cands.hpp>
#include <okiidoku/puzzle2/solver/find_cache.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>

// #include <stack>
#include <vector>
#include <type_traits>

/**
The "engine" is a primitive for building a solver capable of finding all
 solutions to a puzzle. It supports simple solving techniques: subsets,
 fish, and intersections. To enable finding all solutions, the guess
 mechanism is a backtrack-able stack.

I don't expect the average person checking out this library to be interested
 in such tinkering, and there are more usage contracts to follow than there
 are in the rest of the library, so it is not part of the library interface.
*/
namespace okiidoku::mono::detail::solver2 {

	template<Order O> requires(is_order_compiled(O)) struct EngineImpl;

	// usage: must be called immediately when a cell's candidate-symbol count
	//  changes to zero. call to prepare to find another solution.
	// contract: `engine.no_more_solns` returns `false`.
	template<Order O> requires(is_order_compiled(O))
	FindStat unwind_one_stack_frame_of_(EngineImpl<O>&) noexcept;


	template<Order O> requires(is_order_compiled(O)) class CandElimFind;


	template<Order O> requires(is_order_compiled(O))
	struct Guess final {
		int_ts::o4xs_t<O> rmi;
		int_ts::o2xs_t<O> val;
	};


	template<Order O> requires(is_order_compiled(O))
	struct EngineImpl {
		friend FindStat unwind_one_stack_frame_of_<O>(EngineImpl<O>&) noexcept;
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using o4i_t = int_ts::o4i_t<O>;
		using val_t = int_ts::o2xs_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
	public:

		struct Frame final {
			CandsPovs<O> cands_povs;
			FindCacheForSubsets<O> find_cache_subsets;
			FindCacheForFish<O> find_cache_fish;
		};

		// TODO consider a different design: cands_povs_ and num_unsolved_ are just the top
		// entry of the guess_stack_. no_more_solns_ is implied when the guess stack size is zero.
		//  This would make the EngineImpl struct size small enough to probably justify no longer wrapping
		//   Engine with unique_ptr in the Solver classes.
		struct GuessStackFrame final {
			Frame frame;
			Guess<O> guess;
			GuessStackFrame(const Frame& frame_, const Guess<O> guess_) noexcept:
				frame{frame_},
				guess{guess_}
			{}
		};
		using guess_stack_t = std::vector<GuessStackFrame>;
		// using guess_stack_t = std::stack<GuessStackFrame/* , std::vector<GuessStackFrame> */>;
	public:
		EngineImpl() noexcept = default;

		// contract: none. puzzle can even blatantly break the one rule.
		void reinit_with_puzzle(const Grid<O>& puzzle) noexcept;

		// The user of the engine must respond to `get_next_solution` with `std::nullopt`
		// if this returns `true`.
		//
		// Note: All candidate elimination techniques have a contract that this returns `false`.
		// contract: All other non-const member functions require that this return `false`.
		[[nodiscard, gnu::pure]]
		auto no_more_solns() const noexcept { return no_more_solns_; }

		[[nodiscard, gnu::pure]]
		auto get_num_unsolved() const noexcept { return frame_.cands_povs.num_unsolved(); }

		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		void push_guess(Guess<O>) noexcept;

		[[nodiscard, gnu::pure]]
		std::size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); };

		[[nodiscard, gnu::pure]]
		std::uint_fast64_t get_total_guesses() const noexcept { return total_guesses_; };

		// contract: `no_more_solns` returns `false`.
		// contract: `get_num_unsolved` returns zero.
		// returns a filled grid that follows the one rule and contains all the puzzle's givens.
		[[nodiscard, gnu::pure]]
		Grid<O> build_solution_obj() const noexcept;


		[[nodiscard, gnu::pure]] const auto& cands_povs() const noexcept { return frame_.cands_povs; }
		[[nodiscard, gnu::pure]] auto& find_cache_subsets() noexcept { return frame_.find_cache_subsets; }
		[[nodiscard, gnu::pure]] auto& find_cache_fish() noexcept { return frame_.find_cache_fish; }
		[[nodiscard, gnu::pure]] const auto& get_guess_stack_() const noexcept { return guess_stack_; }


		// contract: `val` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: no previous call in context of the current guess stack has been
		//  made with the same value of `rmi`.
		// post-condition: `val` is registered as the only candidate-symbol at `rmi`.
		void register_new_given_(rmi_t rmi, val_t val) noexcept;

		// The specified candidate-symbol is allowed to already be removed.
		FindStat do_elim_remove_sym_(rmi_t rmi, val_t cand) noexcept;

		// The specified candidate-symbols are allowed to already be removed.
		FindStat do_elim_remove_syms_(rmi_t rmi, const O2BitArr<O>& to_remove) noexcept;

	private:
		// The specified candidate-symbol is allowed to already be removed.
		template<class F> requires(std::is_invocable_v<F, O2BitArr<O>&>)
		FindStat do_elim_generic_(rmi_t rmi, F elim_fn) noexcept;

		// contract: must be called immediately when a cell's candidate-symbol count _changes_ to one.
		// contract: (it follows that) no previous call in the context of the current
		//  guess stack has been made with the same value of `rmi`.
		// contract: (it follows that) the cell at `rmi` has exactly one candidate-symbol.
		// post-condition: decrements `num_unsolved`.
		void enqueue_cand_elims_for_new_cell_claim_sym_(rmi_t rmi) noexcept;


		Frame frame_ {
			.cands_povs {},
			.find_cache_subsets {},
			.find_cache_fish {},
		};

		guess_stack_t guess_stack_ {};
		std::uint_fast64_t total_guesses_ {0};
		bool no_more_solns_ {true};
	};


	// Note: The current usage of inheritance (rather than composition) is _only_
	// done to reduce boilerplate in writing the delegating member functions.
	template<Order O> requires(is_order_compiled(O))
	class Engine final : private EngineImpl<O> {
		friend class CandElimFind<O>;
	public:
		// Engine() noexcept = default; // TODO was this ever needed? why was it written?

		// please read the contracts for the referenced functions.
		using EngineImpl<O>::reinit_with_puzzle;
		using EngineImpl<O>::do_elim_remove_sym_;
		using EngineImpl<O>::no_more_solns;
		using EngineImpl<O>::get_num_unsolved;
		using EngineImpl<O>::push_guess;
		using EngineImpl<O>::get_guess_stack_depth;
		using EngineImpl<O>::get_total_guesses;
		using EngineImpl<O>::build_solution_obj;

		// contract: `no_more_solns` returns `false`.
		FindStat unwind_one_stack_frame() noexcept;
	};


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		extern template struct EngineImpl<O_>; \
		extern template class Engine<O_>;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
#endif