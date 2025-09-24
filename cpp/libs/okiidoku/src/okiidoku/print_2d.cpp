// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/print_2d.hpp>
#include <okiidoku/print_2d.emoji.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>
#include <random>      // minstd_rand
#include <tuple>       // apply
#include <array>
#include <string_view> // operator<<
#include <span>
#include <algorithm>
#include <numeric>     // ranges::iota
#include <iterator>    // next, operator+
#include <execution>   // execution::unseq

namespace okiidoku { namespace {

	// current implementation is pretty simple (dumb?)
	void make_random_emoji_set(const rng_seed_t rng_seed, const std::span<std::string_view> sink) noexcept {
		thread_local auto sets_ {emoji::sets};
		thread_local auto sets {std::apply([](auto& ...args)noexcept{
			return std::to_array({(std::span<std::string_view>{args.entries})...});
		}, sets_)};
		std::minstd_rand rng {rng_seed};
		std::shuffle(sets.begin(), sets.end(), rng);
		auto sink_it {sink.begin()};
		for (const auto set : sets) {
			std::shuffle(set.begin(), set.end(), rng);
			sink_it = std::copy(
				OKIIDOKU_UNSEQ
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

	void print_2d_base( // NOLINT(readability-function-cognitive-complexity) :B
		const Order O,
		std::ostream& os,
		const rng_seed_t rng_seed,
		const std::span<const print_2d_grid_view> grid_views
	) noexcept {
		OKIIDOKU_CONTRACT_USE(O <= largest_compiled_order);
		OKIIDOKU_CONTRACT_USE(is_order_compiled(O));
		using o2i_t = visitor::ints::o2i_t;
		const o2i_t O2 {static_cast<o2i_t>(O*O)};

		const auto print_box_row_sep_string_ {[&os, O](const Order border_i) -> void {
			OKIIDOKU_CONTRACT_USE(border_i <= O);
			#define M_NOOK(NOOK_T, NOOK_C, NOOK_B) \
			if      (border_i == 0u) [[unlikely]] { os << (NOOK_T); } \
			else if (border_i == O ) [[unlikely]] { os << (NOOK_B); } \
			else                    { os << (NOOK_C); }
			M_NOOK(" ┌", " ├", " └")
			for (auto box_col {0uL}; box_col < O; ++box_col) {
				for (auto i {0uL}; i < 1uL + (2uL * O); ++i) {
					os << "─";
				}
				if (box_col < O - 1uL) [[likely]] { M_NOOK("┬", "┼", "┴") }
			}
			M_NOOK("┐", "┤", "┘")
			#undef M_NOOK
		}};

		const auto print_box_row_sep_strings {[&](const Order border_i){
			OKIIDOKU_CONTRACT_USE(border_i <= O);
			os << '\n';
			print_box_row_sep_string_(border_i);
			for (std::size_t i {1}; i < grid_views.size(); ++i) {
				os << "   ";
				print_box_row_sep_string_(border_i);
			}
		}};
		std::vector<std::string_view> emoji_set {O2}; // TODO.low consider making thread_local?
		OKIIDOKU_CONTRACT_USE(emoji_set.size() == O2);
		make_random_emoji_set(rng_seed, emoji_set);

		for (o2i_t row {0u}; row < O2; ++row) {
			if (row % O == 0u) [[unlikely]] {
				print_box_row_sep_strings(row / O);
			}
			os << '\n';
			for (std::size_t grid_i {0uz}; grid_i < grid_views.size(); ++grid_i) {
				for (o2i_t col {0u}; col < O2; ++col) {
					if ((col % O) == 0u) [[unlikely]] { os << " │"; }

					o2i_t sym {grid_views[grid_i].operator()(
						static_cast<visitor::ints::o4xs_t>((row * O2) + col)
					)};
					if (sym == O2) {
						os << "  ";
					} else {
						os << emoji_set.at(sym);
					}
				}
				os << " │";
				if (grid_i != grid_views.size() - 1u) {
					os << "   ";
				}
			}
		}
		print_box_row_sep_strings(O);
	}
}