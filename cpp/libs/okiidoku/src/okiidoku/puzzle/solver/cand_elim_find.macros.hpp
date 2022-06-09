#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND__MACROS
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND__MACROS

// yes, I know macros are general recommended against.
// yes, I know macros do not get scoped to namespaces.
// I am not doing this because I like macros.
namespace okiidoku::mono::detail::solver {

	#define OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS \
		using T [[maybe_unused]] = Ints<O>; \
		using o1i_t [[maybe_unused]] = int_ts::o1i_t<O>; \
		using o2xs_t [[maybe_unused]] = int_ts::o2xs_t<O>; \
		using o2x_t [[maybe_unused]] = int_ts::o2x_t<O>; \
		using o2is_t [[maybe_unused]] = int_ts::o2is_t<O>; \
		using o2i_t [[maybe_unused]] = int_ts::o2i_t<O>; \
		using o3xs_t [[maybe_unused]] = int_ts::o3xs_t<O>; \
		using o3i_t [[maybe_unused]] = int_ts::o3i_t<O>; \
		using rmi_t [[maybe_unused]] = int_ts::o4xs_t<O>; \
		using o4i_t [[maybe_unused]] = int_ts::o4i_t<O>;


	#define OKIIDOKU_CAND_ELIM_FINDER_DEF(TECHNIQUE_NAME) \
	template<Order O> requires(is_order_compiled(O)) \
	UnwindInfo CandElimFind<O>::TECHNIQUE_NAME(Engine<O>& engine) noexcept { \
		assert(!engine.no_solutions_remain()); \
		if (engine.get_num_puzcells_remaining() == 0) [[unlikely]] { return UnwindInfo::make_no_unwind(); } \
		const auto needs_unwind {find_ ## TECHNIQUE_NAME ## _and_check_needs_unwind(engine.cells_cands(), engine.get_found_queues_())}; \
		if (needs_unwind) { return engine.unwind_one_stack_frame(); } \
		return UnwindInfo::make_no_unwind(); \
	}


	#define OKIIDOKU_CAND_ELIM_FINDER_DEF_ALT(TECHNIQUE_NAME) \
	template<Order O> requires(is_order_compiled(O)) \
	UnwindInfo CandElimFind<O>::TECHNIQUE_NAME(Engine<O>& engine) noexcept { \
		assert(!engine.no_solutions_remain()); \
		if (engine.get_num_puzcells_remaining() == 0) [[unlikely]] { return UnwindInfo::make_no_unwind(); } \
		return find_ ## TECHNIQUE_NAME ## _and_check_needs_unwind(engine); \
	}
}
#endif