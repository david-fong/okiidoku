// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__ENGINE

#include <okiidoku/puzzle/solver/found_queue.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

// #include <stack>
#include <vector>
#include <array>
#include <type_traits>
#include <cstdint>

/**
The "engine" is a primitive for building a solver capable of finding all
 solutions to a puzzle. It uses a cell-major representation, and implements
 simple solving techniques: subsets, fish, and intersections. To enable
 finding all solutions, the guess mechanism is a backtrack-able stack..

I don't expect the average person checking out this library to be interested
 in such tinkering, and there are more usage contracts to follow than there
 are in the rest of the library, so it is not part of the library interface.
*/
namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O)) struct EngineImpl;

	struct UnwindInfo;

	/**
	usage: must be called immediately when a cell's candidate-symbol count
	changes to zero. call to prepare to find another solution.
	\pre `engine.no_more_solns` returns `false`. */
	template<Order O> requires(is_order_compiled(O))
	UnwindInfo unwind_one_stack_frame_of_(EngineImpl<O>& engine) noexcept;


	struct [[nodiscard]] UnwindInfo {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		friend UnwindInfo unwind_one_stack_frame_of_<O_>(EngineImpl<O_>&) noexcept;
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	private:
		explicit consteval UnwindInfo(bool did_unwind, bool did_unwind_root) noexcept:
			did_unwind_{did_unwind}, did_unwind_root_{did_unwind_root} {}
	public:
		[[nodiscard, gnu::pure]] bool did_unwind() const noexcept { return did_unwind_; }
		[[nodiscard, gnu::pure]] bool did_unwind_root() const noexcept { return did_unwind_root_; }
		UnwindInfo() = delete;
		static constexpr UnwindInfo make_no_unwind() noexcept { return UnwindInfo{false, false}; }
	private:
		static constexpr UnwindInfo make_did_unwind_guess() noexcept { return UnwindInfo{true, false}; }
		static constexpr UnwindInfo make_did_unwind_root() noexcept { return UnwindInfo{true, true}; }

		bool did_unwind_;
		bool did_unwind_root_; // TODO try using bit fields and measure differences.
	};


	template<Order O> requires(is_order_compiled(O)) class CandElimFind;
	template<Order O> requires(is_order_compiled(O)) class CandElimApply;
	template<Order O> requires(is_order_compiled(O)) class CandElimApplyImpl;

	template<Order O> requires(is_order_compiled(O))
	using CandsGrid = detail::Gridlike<O, O2BitArr<O>>;

	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] Guess {
		Ints<O>::o4xs_t rmi;
		Ints<O>::o2xs_t sym;
	};


	// Defines all lower-level operations on engine internals. `Engine` wraps
	// it to enforce appropriate access control to the engine user and to the
	// candidate elimination find and apply operations.
	template<Order O> requires(is_order_compiled(O))
	struct EngineImpl {
		friend UnwindInfo unwind_one_stack_frame_of_<O>(EngineImpl<O>&) noexcept;
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using sym_t = T::o2xs_t;
		using rmi_t = T::o4xs_t;
	public:
		struct HouseSubsets {
			struct [[gnu::designated_init]] CellTag {
				rmi_t rmi;
				Ints<O>::o2is_t count_cache;
			};
			O2BitArr<O> is_begin;
			std::array<CellTag, T::O2> cell_tags; // TODO consider struct of arrays instead
		};
		using houses_subsets_t = HouseTypeMap<
			std::array<HouseSubsets, T::O2>
		>;

		struct [[gnu::designated_init]] Frame {
			o4i_t num_unsolved;
			CandsGrid<O> cells_cands;
			houses_subsets_t houses_subsets;
		};

		// TODO consider a different design: cells_cands_ and num_unsolved_ are just the top
		// entry of the guess_stack_. no_more_solns_ is implied when the guess stack size is zero.
		//  This would make the EngineImpl struct size small enough to probably justify no longer wrapping
		//   Engine with unique_ptr in the Solver classes.
		struct GuessStackFrame {
			Frame frame;
			Guess<O> guess;
			constexpr GuessStackFrame(const Frame& frame_, const Guess<O> guess_) noexcept:
				frame{frame_},
				guess{guess_}
			{}
		};
		using guess_stack_t = std::vector<GuessStackFrame>;
		// using guess_stack_t = std::stack<GuessStackFrame/* , std::vector<GuessStackFrame> */>;
	public:
		EngineImpl() noexcept = default;

		/** \pre none. puzzle can even blatantly break the one rule. */
		void reinit_with_puzzle(const Grid<O>& puzzle) noexcept;

		/**
		The engine wrapper must respond to `get_next_solution` with `std::nullopt`
		if this returns `true`.
		\note all candidate elimination techniques have a contract that this returns `false`.
			all other non-const member functions require that this return `false`. */
		[[nodiscard, gnu::pure]] constexpr
		bool no_more_solns() const noexcept { return no_more_solns_; }

		// the candidate elimination queue is processed in the order of insertion.
		[[nodiscard, gnu::pure]] constexpr
		bool has_queued_cand_elims() const noexcept { return !found_queues_.is_empty(); }


		[[nodiscard, gnu::pure]] constexpr
		o4i_t get_num_unsolved() const noexcept { return frame_.num_unsolved; }

		// contract: `sym` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: only call when `has_queued_cand_elims` returns `false`. There
		//  is _never_ a good reason to make a guess when you have a deduction ready.
		void push_guess(Guess<O>) noexcept;

		[[nodiscard, gnu::pure]] constexpr
		std::size_t get_guess_stack_depth() const noexcept { return guess_stack_.size(); }

		[[nodiscard, gnu::pure]] constexpr
		std::uint_fast64_t get_total_guesses() const noexcept { return total_guesses_; }

		// contract: `no_more_solns` returns `false`.
		// contract: `get_num_unsolved` returns zero.
		// returns a filled grid that follows the one rule and contains all the puzzle's givens.
		[[nodiscard, gnu::pure]]
		Grid<O> build_solution_obj() const noexcept;


		[[nodiscard, gnu::pure]] const auto& cells_cands() const noexcept { return frame_.cells_cands; }
		[[nodiscard, gnu::pure]] auto& houses_subsets() noexcept { return frame_.houses_subsets; }
		[[nodiscard, gnu::pure]] auto& get_found_queues_() noexcept { return found_queues_; }
		[[nodiscard, gnu::pure]] const auto& get_guess_stack_() const noexcept { return guess_stack_; }


		// contract: `sym` is currently one of _multiple_ candidate-symbols at `rmi`.
		// contract: no previous call in context of the current guess stack has been
		//  made with the same value of `rmi`.
		// post-condition: `sym` is registered as the only candidate-symbol at `rmi`.
		void register_new_given_(rmi_t rmi, sym_t sym) noexcept;

		// The specified candidate-symbol is allowed to already be removed.
		UnwindInfo do_elim_remove_sym_(rmi_t rmi, sym_t cand) noexcept;

		// The specified candidate-symbols are allowed to already be removed.
		UnwindInfo do_elim_remove_syms_(rmi_t rmi, const O2BitArr<O>& to_remove) noexcept;

		UnwindInfo do_elim_retain_syms_(rmi_t rmi, const O2BitArr<O>& to_retain) noexcept;

	private:
		[[nodiscard, gnu::pure]] auto& mut_cells_cands() noexcept { return frame_.cells_cands; }

		// The specified candidate-symbol is allowed to already be removed.
		template<class F> requires(std::is_invocable_v<F, O2BitArr<O>&>)
		UnwindInfo do_elim_generic_(rmi_t rmi, F elim_fn) noexcept;

		/**
		\pre must be called immediately when a cell's candidate-symbol count _changes_ to one.
		\pre (it follows that) no previous call in the context of the current
			guess stack has been made with the same value of `rmi`.
		\pre (it follows that) the cell at `rmi` has exactly one candidate-symbol.
		\post decrements `num_unsolved`. */
		void enqueue_cand_elims_for_new_cell_claim_sym_(rmi_t rmi) noexcept;

		OKIIDOKU_KEEP_FOR_DEBUG void debug_print_cells_cands_() const noexcept;
		OKIIDOKU_KEEP_FOR_DEBUG [[nodiscard, gnu::pure]] bool debug_check_correct_num_unsolved_() const noexcept;


		Frame frame_ {
			.num_unsolved {T::O4},
			.cells_cands {},
			.houses_subsets {},
		};

		FoundQueues<O> found_queues_ {};
		guess_stack_t guess_stack_ {};
		std::uint_fast64_t total_guesses_ {0};
		bool no_more_solns_ {true};
	};


	// Note: The current usage of inheritance (rather than composition) is _only_
	// done to reduce boilerplate in writing the delegating member functions.
	template<Order O> requires(is_order_compiled(O))
	class Engine : private EngineImpl<O> {
		friend class CandElimFind<O>;  // the class wraps implementations that can only see what they need.
		friend class CandElimApply<O>;
		friend class CandElimApplyImpl<O>;
	private:
		using T = Ints<O>;
		using sym_t = T::o2xs_t;
		using rmi_t = T::o4xs_t;
	public:
		// Engine() noexcept = default; // TODO was this ever needed? why was it written?

		// please read the contracts for the referenced functions.
		using EngineImpl<O>::reinit_with_puzzle;
		using EngineImpl<O>::do_elim_remove_sym_;
		using EngineImpl<O>::no_more_solns;
		using EngineImpl<O>::has_queued_cand_elims;
		using EngineImpl<O>::get_num_unsolved;
		using EngineImpl<O>::push_guess;
		using EngineImpl<O>::get_guess_stack_depth;
		using EngineImpl<O>::get_total_guesses;
		using EngineImpl<O>::build_solution_obj;

		// contract: `no_more_solns` returns `false`.
		UnwindInfo unwind_one_stack_frame() noexcept;
	};


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		extern template struct EngineImpl<O_>; \
		extern template class Engine<O_>;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
#endif