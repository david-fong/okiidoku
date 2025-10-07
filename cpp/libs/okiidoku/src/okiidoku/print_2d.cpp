// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/print_2d.hpp>
#include <okiidoku/print_2d.emoji.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>
#include <random>      // minstd_rand
#include <vector>
#include <tuple>       // apply, tuple
#include <array>
#include <string_view>
#include <span>
#include <algorithm>   // shuffle, copy, min, sort, unique
#include <iterator>    // distance, next
#include <execution>   // execution::unseq

namespace okiidoku { namespace {

	void make_random_emoji_set(const rng_seed_t rng_seed, const std::span<std::string_view> sink) noexcept {
		thread_local constinit auto sets_ {emoji::sets};
		thread_local static auto sets {std::apply([](auto& ...args)noexcept{
			return std::to_array({(std::span<std::string_view>{args.entries})...});
			// TODO when trying to make `sets` constinit and this lambda consteval, it fails. is it due to span constructor? or because sets_ is constinit and not constexpr?
		}, sets_)};
		std::minstd_rand rng {rng_seed};
		std::shuffle(sets.begin(), sets.end(), rng);
		auto sink_it {sink.begin()};
		for (const auto set : sets) {
			std::shuffle(set.begin(), set.end(), rng);
			sink_it = std::copy(OKIIDOKU_UNSEQ
				set.begin(), // `set.end()` (or fewer, if it would overflow the sink):
				std::next(set.begin(), std::min(std::distance(set.begin(), set.end()), std::distance(sink_it, sink.end()))), // I wish the standard library just handled this or provided a `out.end()` parameter :(
				sink_it
			);
			std::sort(sink.begin(), sink_it); // need to sort before `std::unique`.
			sink_it = std::unique(sink.begin(), sink_it);
			if (sink_it == sink.end()) { break; }
		}
		std::shuffle(sink.begin(), sink.end(), rng);
	}
}}
namespace okiidoku {

	// NOLINTNEXTLINE(*-cognitive-complexity)
	void print_2d_base(
		const Order O,
		std::ostream& os,
		const rng_seed_t rng_seed,
		const std::span<const print_2d_grid_view> grid_views
	) noexcept {
		OKIIDOKU_CONTRACT(O <= largest_compiled_order);
		OKIIDOKU_CONTRACT(is_order_compiled(O));
		using o2i_t = visitor::ints::o2i_t;
		const o2i_t O2 {static_cast<o2i_t>(O*O)};
		if (grid_views.size() == 0uz) [[unlikely]] { return; }

		const auto print_box_row_sep_string_ {[&os, O](const Order border_i) noexcept -> void {
			OKIIDOKU_CONTRACT(border_i <= O);
			#define M_NOOK(NOOK_T, NOOK_C, NOOK_B) /*top,center,bottom*/ \
			if      (border_i == 0u) [[unlikely]] { os << (NOOK_T); } \
			else if (border_i == O ) [[unlikely]] { os << (NOOK_B); } \
			else                                  { os << (NOOK_C); }
			M_NOOK(" ┌", " ├", " └")
			for (auto box_col {0uL}; box_col < O; ++box_col) {
				for (auto i {0uL}; i < 1uL + (2uL * O); ++i) { os << "─"; } // content
				if (box_col < O - 1uL) [[likely]] { M_NOOK("┬", "┼", "┴") } // separator
			}
			M_NOOK("┐", "┤", "┘")
			#undef M_NOOK
		}};
		const auto print_box_row_sep_strings {[&](const Order border_i) noexcept -> void {
			OKIIDOKU_CONTRACT(border_i <= O);
			os << '\n';
			print_box_row_sep_string_(border_i);
			for (auto i {1uz}; i < grid_views.size(); ++i) {
				os << "   ";
				print_box_row_sep_string_(border_i);
			}
		}};
		const auto emoji_set {[&]noexcept{
			std::vector<std::string_view> set {O2};
			OKIIDOKU_CONTRACT2(set.size() == O2);
			make_random_emoji_set(rng_seed, set);
			return set;
		}()};
		OKIIDOKU_CONTRACT2(emoji_set.size() == O2);

		for (o2i_t row {0u}; row < O2; ++row) {
			if (row % O == 0u) [[unlikely]] {
				print_box_row_sep_strings(row / O);
			}
			os << '\n';
			for (auto grid_i {0uz}; grid_i < grid_views.size(); ++grid_i) {
				for (o2i_t col {0u}; col < O2; ++col) {
					if ((col % O) == 0u) [[unlikely]] { os << " │"; }

					const o2i_t sym {grid_views[grid_i].operator()(
						static_cast<visitor::ints::o4xs_t>((row * O2) + col)
					)};
					OKIIDOKU_CONTRACT(sym <= O2);
					if (sym == O2) { os << "  ";              }
					else           { os << emoji_set.at(sym); }
				}
				os << " │";
				if (grid_i != grid_views.size()-1uz) {
					os << "   ";
				}
			}
		}
		print_box_row_sep_strings(O);
	}
}