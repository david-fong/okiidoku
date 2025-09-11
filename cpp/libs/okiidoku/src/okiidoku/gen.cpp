// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/gen.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <random>    // minstd_rand
#include <algorithm> // ranges::count, shuffle
#include <array>
#include <utility>   // swap, forward

namespace okiidoku::mono { namespace {

	using rng_t = std::minstd_rand; // other good LCG parameters: https://arxiv.org/pdf/2001.05304v3.pdf
	namespace stdr = std::ranges;


	template<Order O> requires(is_order_compiled(O))
	struct SymCountsForChuteHouses {
		using T = Ints<O>;
		using o3i_t = T::o3i_t;
		using ch_t  = T::o1x_t;
		using sym_t = T::o2x_t;
		using count_t = typename T::o1is_t;
		// constexpr SymCountsForChuteHouses() noexcept { store_.fill(count_t{0u}); }
		[[nodiscard, gnu::pure]] o3i_t count_num_missing_syms() const noexcept {
			return o3i_t{stdr::count(store_, count_t{0u})};
		}
		template<class Self> [[nodiscard, gnu::pure]]
		auto&& ch_count_sym(this Self&& self, const ch_t ch, const sym_t sym) noexcept {
			ch.check(); sym.check();
			return std::forward<Self>(self).store_[(T::O1*sym) + ch];
		}
	private:
		/**
		each element counts the number of times a symbol occurs in a block.
		shuffling is complete when there are no zeroes. i.e. all counts are `1`.
		- outer dimension (rows) for each of the O2 symbols.
		- inner dimension (cols) for each of the O1 house-chutes.
		laid out this way for cache locality: common operation is symbol moving to
			a different block: decrement and increment.
		\todo experiment with https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size.html */
		std::array<count_t, T::O3> store_ {};
	};

	template<Order O> requires(is_order_compiled(O))
	void make_boxes_valid(Grid<O>& grid, const typename Ints<O>::o2x1_t h_chute, rng_t& rng) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		// std::uintmax_t op_count {0};
		SymCountsForChuteHouses<O> boxes_has {};
		#pragma clang loop unroll(disable)
		for (const auto chute_row : T::O1) {
		for (const auto col : T::O2) {
			auto& count {boxes_has.ch_count_sym(
				col/T::O1,
				*grid.at(h_chute+chute_row, col)
			)};
			++count;
			OKIIDOKU_CONTRACT_USE(count <= T::O1);
		}}
		o3i_t num_missing_syms {boxes_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {(rng() - rng_t::min()) % T::O2};
			const auto b_col {(rng() - rng_t::min()) % T::O2};
			const auto a_box {a_col/T::O1};
			const auto b_box {b_col/T::O1};
			if (a_box == b_box) [[unlikely]] { continue; }
			OKIIDOKU_CONTRACT_USE(a_col != b_col);
			const auto row {h_chute + ((rng() - rng_t::min()) % T::O1)};
			auto& a_sym {grid.at(row,a_col)}; // ↰ will swap these later
			auto& b_sym {grid.at(row,b_col)}; // ↲ so need reference type
			OKIIDOKU_CONTRACT_USE(a_sym != b_sym);
			const auto num_resolved {static_cast<signed char>(
				(boxes_has.ch_count_sym(a_box,*a_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.ch_count_sym(a_box,*b_sym) == 0 ?  1 : 0) + // improvement
				(boxes_has.ch_count_sym(b_box,*b_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.ch_count_sym(b_box,*a_sym) == 0 ?  1 : 0)   // improvement
			)};
			OKIIDOKU_CONTRACT_USE(num_resolved <=  2);
			OKIIDOKU_CONTRACT_USE(num_resolved >= -2);
			if (num_resolved >= 0) [[unlikely]] { // TODO.low for fun: find out on average at what op_count it starts being unlikely
				OKIIDOKU_CONTRACT_USE(num_missing_syms >= detail::Int<2>{num_resolved});
				num_missing_syms -= detail::Int<2>{num_resolved};
				--boxes_has.ch_count_sym(a_box,*a_sym);
				++boxes_has.ch_count_sym(b_box,*a_sym);
				--boxes_has.ch_count_sym(b_box,*b_sym);
				++boxes_has.ch_count_sym(a_box,*b_sym);
				std::swap(a_sym, b_sym);
			}
			// ++op_count;
		}
		// std::cout << "\n" << op_count << ", ";
	}


	template<Order O> requires(is_order_compiled(O))
	void make_cols_valid(Grid<O>& grid, const typename Ints<O>::o2x1_t v_chute, rng_t& rng) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		// std::uintmax_t op_count {0};
		SymCountsForChuteHouses<O> cols_has {};
		#pragma clang loop unroll(disable)
		for (const auto row : T::O2) {
		for (const auto box_col : T::O1) {
			auto& count {cols_has.ch_count_sym(
				box_col,
				*grid.at(row, v_chute + box_col)
			)};
			++count;
			OKIIDOKU_CONTRACT_USE(count <= T::O1);
		}}
		o3i_t num_missing_syms {cols_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {(rng() - rng_t::min()) % T::O1};
			const auto b_col {(rng() - rng_t::min()) % T::O1};
			if (a_col == b_col) [[unlikely]] { continue; }
			const auto row {(rng() - rng_t::min()) % T::O2};
			auto& a_sym {grid.at(row, v_chute + a_col)}; // ↰ will swap these later
			auto& b_sym {grid.at(row, v_chute + b_col)}; // ↲ so need reference type
			OKIIDOKU_CONTRACT_USE(a_sym != b_sym);
			const auto num_resolved {static_cast<signed char>(
				(cols_has.ch_count_sym(a_col,*a_sym) == 1 ? -1 : 0) + // regression
				(cols_has.ch_count_sym(a_col,*b_sym) == 0 ?  1 : 0) + // improvement
				(cols_has.ch_count_sym(b_col,*b_sym) == 1 ? -1 : 0) + // regression
				(cols_has.ch_count_sym(b_col,*a_sym) == 0 ?  1 : 0)   // improvement
			)};
			OKIIDOKU_CONTRACT_USE(num_resolved <=  2);
			OKIIDOKU_CONTRACT_USE(num_resolved >= -2);
			if (num_resolved >= 0) [[unlikely]] {
				OKIIDOKU_CONTRACT_USE(num_missing_syms >= detail::Int<2>{num_resolved});
				num_missing_syms -= detail::Int<2>{num_resolved};
				--cols_has.ch_count_sym(a_col,*a_sym);
				++cols_has.ch_count_sym(a_col,*b_sym);
				--cols_has.ch_count_sym(b_col,*b_sym);
				++cols_has.ch_count_sym(b_col,*a_sym);
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
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		// TODO.low assert that rows follow the rule.

		rng_t rng {rng_seed};
		// #pragma clang loop unroll(disable)
		for (const auto row : T::O2) {
			const auto row_sp {grid.row_span_at(row)};
			std::shuffle(row_sp.begin(), row_sp.end(), rng);
		}
		/* Note: when making boxes valid, keeping one line untouched works,
		but is actually slower. same for making columns valid and one box. */

		// Note: iterations _can_ safely run concurrently.
		for (const auto h_chute : T::O1) {
			make_boxes_valid(grid, h_chute * T::O1, rng);
		}
		// Note: iterations _can_ safely run concurrently, but cache-write-back might
		// get hairy due to vertical chutes "interleaving" in the row-major grid.
		for (const auto v_chute : T::O1) {
			make_cols_valid(grid, v_chute * T::O1, rng);
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule<O>(grid));
	}

	template<Order O> requires(is_order_compiled(O))
	Grid<O> generate_shuffled(rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		OKIIDOKU_DEFER_INIT Grid<O> grid;
		for (const auto row : T::O2) {
		for (const auto col : T::O2) {
			grid.at(row, col) = col;
		}}
		generate_shuffled<O>(grid, rng_seed);
		return grid;
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

	Grid generate_shuffled(const Order O, const rng_seed_t rng_seed) noexcept {
		switch (O) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::generate_shuffled<O_>(rng_seed);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}