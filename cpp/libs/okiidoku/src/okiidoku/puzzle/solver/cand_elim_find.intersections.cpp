#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/fold.hpp>

#include <functional> // function, bit_or
#include <algorithm>
#include <array>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	template<Order O> requires(is_order_compiled(O))
	struct ChuteIsecsSyms final {
		using T = Ints<O>;
		using o1i_t = int_ts::o1i_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		// inner dimension is for intersections in a line, outer dimension for intersections in a box.
		[[nodiscard, gnu::pure]] const O2BitArr<O>& at_isec(const o2i_t isec_i) const noexcept { return arr_[isec_i]; }
		[[nodiscard, gnu::pure]]       O2BitArr<O>& at_isec(const o2i_t isec_i)       noexcept { return arr_[isec_i]; }
		[[nodiscard, gnu::pure]] const O2BitArr<O>& at_isec(const o1i_t box_isec_i, const o1i_t line_isec_i) const noexcept { return arr_[(T::O1*box_isec_i)+line_isec_i]; }
		[[nodiscard, gnu::pure]]       O2BitArr<O>& at_isec(const o1i_t box_isec_i, const o1i_t line_isec_i)       noexcept { return arr_[(T::O1*box_isec_i)+line_isec_i]; }
	private:
		std::array<O2BitArr<O>, T::O2> arr_ {};
	};

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_locked_cands_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		(void)cells_cands, (void)found_queues;// TODO
		// for intersection I of block B and line L, and symbol S,
		// if S's only candidate cells in L are in I, the same must hold true for B
		// if S's only candidate cells in B are in I, the same must hold true for L
		const auto line_type {LineType::row};
		for (o1i_t chute {0}; chute < T::O1; ++chute) {
			ChuteIsecsSyms<O> chute_isecs_syms {};
			for (o3i_t chute_cell {0}; chute_cell < T::O3; ++chute_cell) {
				const auto chute_isec {static_cast<o2x_t>(chute_cell/T::O1)};
				const auto rmi {chute_cell_to_rmi<O>(line_type, chute, chute_cell)};
				chute_isecs_syms.at_isec(chute_isec) |= cells_cands.at_rmi(rmi);
			}
			// for each isec in line, find syms that only have one candidate isec
			for (o1i_t house {0}; house < T::O1; ++house) {
			for (o1i_t house_isec {0}; house_isec < T::O1; ++house_isec) {
				// if ()
			}}
		}
		return false;
	}
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(locked_cands)
	#undef OKIIDOKU_CAND_ELIM_FINDER_DEF

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template UnwindInfo CandElimFind<O_>::locked_cands(Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}