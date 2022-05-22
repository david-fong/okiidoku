#include <okiidoku/puzzle/solver/cand_elim_apply.hpp>

#include <algorithm>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain CandElimApply<O>::apply(
		EngineObj<O>& engine,
		const cand_elim_desc::CellRequiresSymbol<O>& desc // TODO consider/try passing by value
	) noexcept {
		// repetitive code. #undef-ed before end of function.
		#define OKIIDOKU_TRY_ELIM_NB_CAND \
			if (nb_rmi == desc.rmi) [[unlikely]] { continue; } \
			const auto check {engine.cell_elim_cand_sym_(nb_rmi, desc.val)}; \
			if (check.no_solutions_remain()) [[unlikely]] { return check; }

		// TODO consider using the new house_cell_to_rmi function and house_types array.
		{
			const auto desc_row {rmi_to_row<O>(desc.rmi)};
			for (o2i_t nb_col {0}; nb_col < T::O2; ++nb_col) {
				const auto nb_rmi {static_cast<rmi_t>((T::O2*desc_row)+nb_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_col {rmi_to_col<O>(desc.rmi)};
			for (o2i_t nb_row {0}; nb_row < T::O2; ++nb_row) {
				const auto nb_rmi {static_cast<rmi_t>((T::O2*nb_row)+desc_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_box {rmi_to_box<O>(desc.rmi)};
			for (o2i_t nb_box_cell {0}; nb_box_cell < T::O2; ++nb_box_cell) {
				const auto nb_rmi {static_cast<rmi_t>(box_cell_to_rmi<O>(desc_box, nb_box_cell))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		#undef OKIIDOKU_TRY_ELIM_NB_CAND
		return SolutionsRemain::yes();
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain CandElimApply<O>::apply(
		EngineObj<O>& engine,
		const cand_elim_desc::SymbolRequiresCell<O>& desc
	) noexcept {
		auto& cell_cands {engine.cells_cands_.at_rmi(desc.rmi)};
		assert(cell_cands.test(desc.val));
		if (cell_cands.count() > 1) {
			engine.register_new_given_(desc.rmi, desc.val);
		}
		return SolutionsRemain::yes();
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain CandElimApply<O>::apply(
		EngineObj<O>& engine,
		const cand_elim_desc::CellsRequireSymbols<O>& desc
	) noexcept {
		for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
			// TODO likelihood attribute. hypothesis: desc.house_cells.count() is small. please empirically test.
			if (desc.house_cells.test(static_cast<o2x_t>(house_cell))) [[unlikely]] { continue; }
			const auto rmi {house_cell_to_rmi<O>(desc.house_type, desc.house, house_cell)};
			const auto check {engine.cell_elim_cand_syms_(static_cast<rmi_t>(rmi), desc.syms)};
			if (check.no_solutions_remain()) [[unlikely]] {
				return check;
			}
		}
		return SolutionsRemain::yes();
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain CandElimApply<O>::apply(
		EngineObj<O>& engine,
		const cand_elim_desc::SymbolsRequireCells<O>& desc
	) noexcept {
		// TODO.wait HouseMask<O>::set_bits_iter <- create and use instead.
		for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
			// TODO likelihood attribute. hypothesis: desc.house_cells.count() is small. please empirically test.
			if (desc.house_cells.test(static_cast<o2x_t>(house_cell))) [[unlikely]] {
				const auto rmi {house_cell_to_rmi<O>(desc.house_type, desc.house, house_cell)};
				const auto check {engine.cell_elim_cand_syms_(static_cast<rmi_t>(rmi), desc.syms)};
				if (check.no_solutions_remain()) [[unlikely]] {
					return check;
				}
			}
		}
		return SolutionsRemain::yes();
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimApply<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}