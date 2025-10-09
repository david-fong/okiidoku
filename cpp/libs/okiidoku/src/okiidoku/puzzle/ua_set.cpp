// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/ua_set.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <array>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono { namespace {

	// outer dimension for lines of chute.
	// inner dimension maps syms to house-cells.
	template<Order O> requires(is_order_compiled(O))
	using chute_lines_sym_to_cell_t = std::array<
		std::array<typename Ints<O>::o2xs_t, Ints<O>::O2>,
		Ints<O>::O1
	>;

	template<Order O> requires(is_order_compiled(O))
	void find_size_4_minimal_unavoidable_sets_in_chute(
		const Grid<O>& soln_grid,
		MinimalUnavoidableSets<O>& found,
		const LineType line_type,
		const typename Ints<O>::o1x_t chute
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT(chute < T::O1);
		const auto chute_lines_sym_to_cell {[&][[gnu::pure]](){
			chute_lines_sym_to_cell_t<O> map OKIIDOKU_DEFER_INIT;
			for (const auto chute_line : T::O1) {
			for (const auto house_cell : T::O2) {
				const auto chute_cell_i {(T::O2*chute_line)+house_cell};
				const auto rmi {chute_cell_to_rmi<O>(line_type, chute, chute_cell_i)};
				const auto& sym {soln_grid[rmi]};
				OKIIDOKU_CONTRACT(sym < T::O2);
				map[chute_line][sym] = house_cell;
			}}
			return map;
		}()};
		for (o1i_t chute_line_a {0u}; chute_line_a.next() < T::O1; ++chute_line_a) {
		for (o1i_t chute_line_b {chute_line_a.next()}; chute_line_b < T::O1; ++chute_line_b) {
			for (const auto slice_c : T::O2) {
				const auto c_a_rmi {chute_cell_to_rmi<O>(line_type, chute, (T::O2*(*chute_line_a))+slice_c)};
				const auto c_b_rmi {chute_cell_to_rmi<O>(line_type, chute, (T::O2*(*chute_line_b))+slice_c)};
				const auto& c_a_sym {soln_grid[c_a_rmi]}; OKIIDOKU_CONTRACT(c_a_sym < T::O2);
				const auto& c_b_sym {soln_grid[c_b_rmi]}; OKIIDOKU_CONTRACT(c_b_sym < T::O2);
				const auto& d_a_cell {chute_lines_sym_to_cell[chute_line_a][c_b_sym]};
				const auto& d_b_cell {chute_lines_sym_to_cell[chute_line_b][c_a_sym]};
				if ((d_a_cell == d_b_cell) && (d_a_cell > slice_c)) [[unlikely]] {
					found.ua_set_4s.emplace_back(UaSet4<O>{
						c_a_rmi,
						c_b_rmi,
						chute_cell_to_rmi<O>(line_type, chute, (T::O2*(*chute_line_a))+d_a_cell),
						chute_cell_to_rmi<O>(line_type, chute, (T::O2*(*chute_line_b))+d_b_cell),
					});
				}
			}
		}}
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	MinimalUnavoidableSets<O> find_size_4_minimal_unavoidable_sets(const Grid<O>& soln_grid) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		MinimalUnavoidableSets<O> found {}; // TODO actually populate with found UA sets.
		found.ua_set_4s.reserve(1u << (O+2u));
		for (const auto line_type : line_types) {
			for (const auto chute : T::O1) {
				find_size_4_minimal_unavoidable_sets_in_chute(
					soln_grid, found, line_type, chute
				);
			}
		}
		return found;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template MinimalUnavoidableSets<(O_)> find_size_4_minimal_unavoidable_sets<(O_)>(const Grid<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}