// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/gen.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <random>    // minstd_rand
#include <algorithm> // count, shuffle
#include <array>
#include <utility>   // swap, forward

namespace okiidoku::mono { namespace {

	using rng_t = std::minstd_rand; // other good LCG parameters: https://arxiv.org/pdf/2001.05304v3.pdf


	template<Order O> requires(is_order_compiled(O))
	struct SymCountsForChuteHouses {
		using T = Ints<O>;
		using V = T::o1i_t;
		using o3i_t = T::o3i_t;
		using ch_t = T::o1x_t;
		using sym_t = T::o2i_t;
		[[nodiscard, gnu::pure]] o3i_t count_num_missing_syms() const noexcept {
			return T::o3i(std::count(store_.cbegin(), store_.cend(), V{0}));
		}
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& ch_count_sym(this Self&& self, const ch_t ch, const sym_t sym) noexcept {
			OKIIDOKU_CONTRACT_USE(ch < T::O1);
			OKIIDOKU_CONTRACT_USE(sym < T::O2);
			return std::forward<Self>(self).store_[T::o3i(T::o3i(T::O1*sym)+ch)];
		}
	private:
		// outer dimension (rows) for each of the O2 symbols
		// inner dimension (cols) for each of the O1 house-chutes.
		std::array<V, T::O3> store_ {0};
	};

	template<Order O> requires(is_order_compiled(O))
	void make_boxes_valid(Grid<O>& grid, const typename Ints<O>::o2x_t h_chute, rng_t& rng) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(h_chute <= T::O2-T::O1);

		// unsigned long long op_count {0};
		SymCountsForChuteHouses<O> boxes_has {};
		#pragma clang loop unroll(disable)
		for (o2i_t row {h_chute}; row < h_chute+T::O1; ++row) {
		for (o2i_t col {0}; col < T::O2; ++col) {
			auto& count {boxes_has.ch_count_sym(
				T::o1x(col/T::O1),
				grid.at(row,col)
			)};
			++count;
			OKIIDOKU_CONTRACT_USE(count <= T::O1);
		}}
		o3i_t num_missing_syms {boxes_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {T::o2x((rng() - rng_t::min()) % T::O2)};
			const auto b_col {T::o2x((rng() - rng_t::min()) % T::O2)};
			const auto a_box {T::o1x(a_col/T::O1)};
			const auto b_box {T::o1x(b_col/T::O1)};
			if (a_box == b_box) [[unlikely]] { continue; }
			OKIIDOKU_CONTRACT_USE(a_col != b_col);
			const auto row {T::o2x(h_chute + ((rng() - rng_t::min()) % T::O1))};
			auto& a_sym {grid.at(row,a_col)};
			auto& b_sym {grid.at(row,b_col)};
			OKIIDOKU_CONTRACT_USE(&a_sym != &b_sym);
			OKIIDOKU_CONTRACT_USE(a_sym != b_sym);
			const auto num_resolved {static_cast<signed char>(
				(boxes_has.ch_count_sym(a_box,a_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.ch_count_sym(a_box,b_sym) == 0 ?  1 : 0) + // improvement
				(boxes_has.ch_count_sym(b_box,b_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.ch_count_sym(b_box,a_sym) == 0 ?  1 : 0)   // improvement
			)};
			OKIIDOKU_CONTRACT_USE(num_resolved <=  2);
			OKIIDOKU_CONTRACT_USE(num_resolved >= -2);
			if (num_resolved >= 0) [[unlikely]] { // TODO.low for fun: find out on average at what op_count it starts being unlikely
				OKIIDOKU_CONTRACT_USE(num_missing_syms >= T::o3i(num_resolved));
				num_missing_syms = T::o3i(num_missing_syms - T::o3i(num_resolved));
				--boxes_has.ch_count_sym(a_box,a_sym);
				++boxes_has.ch_count_sym(a_box,b_sym);
				--boxes_has.ch_count_sym(b_box,b_sym);
				++boxes_has.ch_count_sym(b_box,a_sym);
				std::swap(a_sym, b_sym);
			}
			// ++op_count;
		}
		// std::cout << "\n" << op_count << ", ";
	}


	template<Order O> requires(is_order_compiled(O))
	void make_cols_valid(Grid<O>& grid, const typename Ints<O>::o2x_t v_chute, rng_t& rng) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(v_chute <= T::O2-T::O1);

		// unsigned long long op_count {0};
		SymCountsForChuteHouses<O> cols_has {};
		#pragma clang loop unroll(disable)
		for (o2i_t row {0}; row < T::O2; ++row) {
		for (o1i_t box_col {0}; box_col < T::O1; ++box_col) {
			auto& count {cols_has.ch_count_sym(
				T::o1x(box_col),
				grid.at(row, T::o2x(v_chute + box_col))
			)};
			++count;
			OKIIDOKU_CONTRACT_USE(count <= T::O1);
		}}
		o3i_t num_missing_syms {cols_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {T::o1x((rng() - rng_t::min()) % T::O1)};
			const auto b_col {T::o1x((rng() - rng_t::min()) % T::O1)};
			if (a_col == b_col) [[unlikely]] { continue; }
			const auto row {T::o2x((rng() - rng_t::min()) % (T::O2))};
			auto& a_sym {grid.at(row, T::o2x(v_chute + a_col))};
			auto& b_sym {grid.at(row, T::o2x(v_chute + b_col))};
			OKIIDOKU_CONTRACT_USE(&a_sym != &b_sym);
			OKIIDOKU_CONTRACT_USE(a_sym != b_sym);
			const auto num_resolved {static_cast<signed char>(
				(cols_has.ch_count_sym(a_col,a_sym) == 1 ? -1 : 0) + // regression
				(cols_has.ch_count_sym(a_col,b_sym) == 0 ?  1 : 0) + // improvement
				(cols_has.ch_count_sym(b_col,b_sym) == 1 ? -1 : 0) + // regression
				(cols_has.ch_count_sym(b_col,a_sym) == 0 ?  1 : 0)   // improvement
			)};
			OKIIDOKU_CONTRACT_USE(num_resolved <=  2);
			OKIIDOKU_CONTRACT_USE(num_resolved >= -2);
			if (num_resolved >= 0) [[unlikely]] {
				OKIIDOKU_CONTRACT_USE(num_missing_syms >= T::o3i(num_resolved));
				num_missing_syms = T::o3i(num_missing_syms - T::o3i(num_resolved));
				--cols_has.ch_count_sym(a_col,a_sym);
				++cols_has.ch_count_sym(a_col,b_sym);
				--cols_has.ch_count_sym(b_col,b_sym);
				++cols_has.ch_count_sym(b_col,a_sym);
				std::swap(a_sym, b_sym);
			}
			// ++op_count;
		}
		// std::cout << op_count;
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void generate_shuffled(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		using o2i_t = T::o2i_t;
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		// TODO.low assert that rows follow the rule.

		rng_t rng {rng_seed};
		// #pragma clang loop unroll(disable)
		for (o2i_t row {0}; row < T::O2; ++row) {
			const auto row_sp {grid.row_span_at(row)};
			std::shuffle(row_sp.begin(), row_sp.end(), rng);
		}
		/* Note: when making boxes valid, keeping one line untouched works,
		but is actually slower. same for making columns valid and one box. */

		// Note: iterations _can_ safely run concurrently.
		for (o2i_t h_chute {0}; h_chute < T::O2; h_chute += T::O1) {
			make_boxes_valid(grid, T::o2x(h_chute), rng);
		}
		// Note: iterations _can_ safely run concurrently, but cache-write-back might
		// get hairy due to vertical chutes "interleaving" in the row-major grid.
		for (o2i_t v_chute {0}; v_chute < T::O2; v_chute += T::O1) {
			make_cols_valid(grid, T::o2x(v_chute), rng);
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule<O>(grid));
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template void generate_shuffled<O_>(Grid<O_>&, rng_seed_t) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	void generate_shuffled(Grid& vis_sink, const rng_seed_t rng_seed) noexcept {
		switch (vis_sink.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::generate_shuffled(vis_sink.unchecked_get_mono_exact<O_>(), rng_seed);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}