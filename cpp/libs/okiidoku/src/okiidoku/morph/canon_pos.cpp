// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/detail/canon_stages.hpp>

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/morph/canon_ties.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <range/v3/view/common.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/algorithm/lexicographical_compare.hpp>

#include <algorithm> // sort
#include <numeric>   // ranges::iota
#include <iterator>  // next
#include <array>

namespace okiidoku::mono { namespace {

	template<Order O>
	using line_map_t = typename Transformation<O>::line_map_t;


	template<Order O> requires(is_order_compiled(O))
	class CanonPlace final {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using sym_t = T::o2is_t;
		using to_t = typename Transformation<O>::to_t;

		struct PolarState final {
			line_map_t<O> to_og {Transformation<O>{}.row_map};
			detail::Ties<O,2> line_ties {};
			detail::Ties<O,1> chute_ties {};

			explicit PolarState() noexcept {
				line_ties.update([][[gnu::const]](auto a, auto b)noexcept{
					return (a%T::O1) == (b%T::O1);
				});
			}
			[[nodiscard, gnu::pure]] bool has_ties() const noexcept {
				return line_ties.has_unresolved() || chute_ties.has_unresolved();
			}

			void do_a_pass(const Grid<O>& table) noexcept;
		};
		static Grid<O> make_table_for_a_pass(const Grid<O>& src_grid, bool is_post_transpose, const PolarState& row, const PolarState& col) noexcept;

	public:
		static Transformation<O> do_it(Grid<O>& src_grid) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	Grid<O> CanonPlace<O>::make_table_for_a_pass(
		const Grid<O>& src_grid,
		const bool is_post_transpose,
		const PolarState& row_state,
		const PolarState& col_state
	) noexcept {
		Grid<O> table OKIIDOKU_DEFER_INIT; {
			const Transformation<O> t {
				.row_map {row_state.to_og},
				.col_map {col_state.to_og},
				.post_transpose {is_post_transpose},
			};
			t.inverted().apply_from_to(src_grid, table);
		}

		for (const auto row_i : T::O2) {
			const auto row_sp {table.row_span_at(row_i)};
			const auto& ortho {is_post_transpose ? row_state : col_state};
			// loop over orthogonal partially-resolved line ranges to normalize:
			for (const auto [t_begin, t_end] : ortho.line_ties) {
				std::sort(std::next(row_sp.begin(), t_begin), std::next(row_sp.begin(), t_end));
			}
			// loop over orthogonal partially-resolved chute ranges to normalize:
			{
				std::array<o1i_t, T::O1> resolve; // TODO.high where is this being used?
				std::iota(resolve.begin(), resolve.end(), o1i_t{0u});
				for (const auto t : ortho.chute_ties) {
					ranges::sort(
						std::next(resolve.begin(), t.begin_),
						std::next(resolve.begin(), t.end_),
						[&][[gnu::pure]](auto a, auto b){
							return ranges::lexicographical_compare(row_sp.subspan(a*T::O1,T::O1), row_sp.subspan(b*T::O1,T::O1));
						}
					);
				}
				std::array<sym_t, T::O2> copy OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
				std::copy(row_sp.begin(), row_sp.end(), copy.begin());
				for (const auto i : T::O1) {
					std::copy(
						std::next(copy.begin(), i*T::O1), std::next(copy.begin(), (i.next())*T::O1),
						std::next(row_sp.begin(), i*T::O1)
					);
				}
			}
		}
		return table;
	}


	template<Order O> requires(is_order_compiled(O))
	void CanonPlace<O>::PolarState::do_a_pass(const Grid<O>& table) noexcept {
		std::array<to_t, T::O2> to_tied OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
		std::iota(to_tied.begin(), to_tied.end(), to_t{0u});
		for (const auto tie : line_ties) {
			tie.check();
			// Note: intentionally do not skip ties here since updated table
			// rows could (and likely will) be used by chute tie resolution.

			// try to resolve tied line range:
			std::sort(
				std::next(to_tied.begin(), tie.begin_),
				std::next(to_tied.begin(), tie.end_),
				[&][[gnu::pure]](auto a, auto b){ return ranges::lexicographical_compare(table.row_span_at(a), table.row_span_at(b)); }
			);
		}
		const auto chute_tie_data {[&][[gnu::pure]](o2i_t chute) {
			namespace v = ::ranges::views;
			return to_tied
				| v::drop((chute*T::O1).val())
				| v::take(T::O1.val())
				| v::transform([&][[gnu::pure]](auto i){ return v::common(table.row_span_at(i)); }) | v::join;
		}};
		// try to resolve tied chute ranges:
		for (const auto tie : chute_ties) {
			tie.check();
			std::sort(
				std::next(to_tied.begin(), tie.begin_), // TODO.high THIS IS WRONG.
				std::next(to_tied.begin(), tie.end_),
				[&](auto a, auto b){ return ranges::lexicographical_compare(
					chute_tie_data(a), chute_tie_data(b)
				); }
			);
		}

		line_ties.update([&][[gnu::pure]](o2xs_t a, o2xs_t b){
			return ranges::equal(table.row_span_at(to_tied[a]), table.row_span_at(to_tied[b]));
		});
		chute_ties.update([&][[gnu::pure]](o1x_t a, o1x_t b){
			return ranges::equal(chute_tie_data(a), chute_tie_data(b));
		});
		// TODO.high tie data for lines in chute currently are not updated after updates to chute ordering...

		{
			// update s.to_og:
			std::array<to_t, T::O2> tied_to_og OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
			for (const auto i : T::O2) {
				tied_to_og[i] = to_og[i/T::O1][i%T::O1];
			}
			for (const auto i : T::O2) {
				to_og[i/T::O1][i%T::O1] = tied_to_og[to_tied[i]];
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	Transformation<O> CanonPlace<O>::do_it(Grid<O>& src_grid) noexcept {
		PolarState row_state {};
		PolarState col_state {};

		while (row_state.has_ties() || col_state.has_ties()) {
			const auto old_row_state {row_state};
			const auto old_col_state {col_state};
			{ auto table_ {make_table_for_a_pass(src_grid, false, old_row_state, old_col_state)}; row_state.do_a_pass(table_); }
			{ auto table_ {make_table_for_a_pass(src_grid, true,  old_row_state, old_col_state)}; col_state.do_a_pass(table_); }

			if (  old_row_state.line_ties  == row_state.line_ties
				&& old_row_state.chute_ties == row_state.chute_ties
				&& old_col_state.line_ties  == col_state.line_ties
				&& old_col_state.chute_ties == col_state.chute_ties
			) [[unlikely]] {
				// TODO.mid stalemate... current design insufficient?
				break;
			}
			// polar state A has no chance for further resolution in the next round
			// if polar state B was unable to resolve any ties in this round.
		}

		Transformation<O> transformation {
			.row_map {row_state.to_og},
			.col_map {col_state.to_og},
			.post_transpose {false},
		};
		transformation = transformation.inverted();
		// TODO.high use two iota views mapped one to src_grid and one to post_transposed view and lexicographical compare. if post_transposed less, edit transformation and apply a post_transpose_only transformation to src_grid in place.
		transformation.apply_in_place(src_grid);
		OKIIDOKU_ASSERT(src_grid.follows_rule());
		return transformation;
	}
}}
namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canon_pos(Grid<O>& grid) noexcept {
		return CanonPlace<O>::do_it(grid);
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template Transformation<(O_)> canon_pos<(O_)>(Grid<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}