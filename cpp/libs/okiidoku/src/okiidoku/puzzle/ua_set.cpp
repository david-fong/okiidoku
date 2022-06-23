#include <okiidoku/puzzle/ua_set.hpp>

#include <okiidoku/detail/contract.hpp>

#include <array>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono { namespace {

	// outer dimension for lines of chute.
	// inner dimension maps syms to house-cells.
	template<Order O> requires(is_order_compiled(O))
	using chute_lines_sym_to_cell_t = std::array<
		std::array<int_ts::o2xs_t<O>, Ints<O>::O2>,
		Ints<O>::O1
	>;

	template<Order O> requires(is_order_compiled(O))
	void find_size_4_minimal_unavoidable_sets_in_chute(
		const Grid<O>& soln_grid,
		const LineType line_type,
		const int_ts::o1i_t<O> chute
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(chute < T::O1);
		const auto chute_lines_sym_to_cell {[&](){
			chute_lines_sym_to_cell_t<O> map;
			for (o1i_t chute_line {0}; chute_line < T::O1; ++chute_line) {
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto chute_cell_i {static_cast<o3i_t>((T::O2*chute_line)+house_cell)};
				OKIIDOKU_CONTRACT_TRIVIAL_EVAL(chute_cell_i < T::O3);
				const auto rmi {chute_cell_to_rmi<O>(line_type, chute, chute_cell_i)};
				const auto& sym {soln_grid.at_rmi(rmi)};
				OKIIDOKU_CONTRACT_TRIVIAL_EVAL(sym < T::O2);
				map[chute_line][sym] = static_cast<o2xs_t>(house_cell);
			}}
			return map;
		}()};
		for (o1i_t chute_line_a {0}; chute_line_a < T::O1; ++chute_line_a) {
		for (o1i_t chute_line_b {0}; chute_line_b < T::O1; ++chute_line_b) {
			for (o2i_t slice_c {0}; slice_c < T::O2; ++slice_c) {
				const auto c_a_rmi {chute_cell_to_rmi<O>(line_type, chute, static_cast<o3i_t>((T::O2*chute_line_a)+slice_c))};
				const auto c_b_rmi {chute_cell_to_rmi<O>(line_type, chute, static_cast<o3i_t>((T::O2*chute_line_b)+slice_c))};
				const auto& c_a_sym {soln_grid.at_rmi(c_a_rmi)}; OKIIDOKU_CONTRACT_TRIVIAL_EVAL(c_a_rmi < T::O2);
				const auto& c_b_sym {soln_grid.at_rmi(c_b_rmi)}; OKIIDOKU_CONTRACT_TRIVIAL_EVAL(c_b_rmi < T::O2);
				const auto& d_a_cell {chute_lines_sym_to_cell[chute_line_a][c_b_sym]};
				const auto& d_b_cell {chute_lines_sym_to_cell[chute_line_b][c_a_sym]};
				if (d_a_cell == d_b_cell) [[unlikely]] {
					; // TODO
				}
			}
		}}
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	MinimalUnavoidableSets<O> find_size_4_minimal_unavoidable_sets(const Grid<O>& soln_grid) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		MinimalUnavoidableSets<O> found; // TODO actually populate with found UA sets.
		for (const auto line_type : line_types) {
			for (o1i_t chute {0}; chute < T::O1; ++chute) {
				find_size_4_minimal_unavoidable_sets_in_chute(soln_grid, line_type, chute);
			}
		}
		return found;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template MinimalUnavoidableSets<O_> find_size_4_minimal_unavoidable_sets<O_>(const Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}