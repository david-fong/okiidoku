// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/print_2d.hpp>
#include <okiidoku/print_2d.emoji.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>
#include <random>      // minstd_rand
#include <vector>
#include <string_view> // operator<<
#include <algorithm>
#include <numeric>     // ranges::iota
#include <iterator>    // next, operator+

namespace okiidoku { namespace {

	// current implementation is pretty simple (dumb?)
	std::vector<size_t> make_random_emoji_set(const Order O, const rng_seed_t rng_seed) noexcept {
		const auto O2 {static_cast<okiidoku::visitor::ints::o2i_t>(O*O)};
		const auto& prefs {emoji::top_set_preferences};
		std::vector<size_t> shuffled_sets(emoji::sets.size());
		std::iota(shuffled_sets.begin(), shuffled_sets.end(), size_t{0});
		{
			using rng_t = std::minstd_rand; // other good LCG parameters: https://arxiv.org/pdf/2001.05304v3.pdf
			rng_t rng {rng_seed};
			for (size_t b {0}, e_i_ {0}; b != prefs.back(); ++e_i_) {
				const size_t e {prefs[e_i_]};
				std::shuffle(
					std::next(shuffled_sets.begin(), static_cast<long>(b)),
					std::next(shuffled_sets.begin(), static_cast<long>(e)),
					rng
				);
				b = e;
			}
		}
		// first try to find a single set large enough:
		// unlike before, don't use anything not in top prefs list here.
		for (size_t b {0}, e_i_ {0}; e_i_ < prefs.size(); ++e_i_) {
			const size_t e {prefs[e_i_]};
			for (size_t i {b}; i < e; ++i) {
				if (emoji::sets.at(shuffled_sets.at(i)).entries.size() >= O2) {
					shuffled_sets[0] = shuffled_sets[i];
					shuffled_sets.resize(1);
					return shuffled_sets;
			}	}
			b = e;
		}
		// otherwise just return everything:
		return shuffled_sets;
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
			if      (border_i == 0) [[unlikely]] { os << (NOOK_T); } \
			else if (border_i == O) [[unlikely]] { os << (NOOK_B); } \
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
		const auto emoji_sets {make_random_emoji_set(O, rng_seed)};

		for (o2i_t row {0}; row < O2; ++row) {
			if (row % O == 0) [[unlikely]] {
				print_box_row_sep_strings(row / O);
			}
			os << '\n';
			for (std::size_t grid_i {0}; grid_i < grid_views.size(); ++grid_i) {
				for (o2i_t col {0}; col < O2; ++col) {
					if ((col % O) == 0) [[unlikely]] { os << " │"; }

					auto val {o2i_t{grid_views[grid_i].operator()(
						static_cast<visitor::ints::o4xs_t>((row * O2) + col)
					)}};
					if (val == O2) {
						os << "  ";
						continue;
					}
					for (const auto emoji_set_index : emoji_sets) {
						const auto& set {emoji::sets.at(emoji_set_index).entries};
						if (val < set.size()) {
							os << set.at(val);
							break;
						}
						val -= static_cast<o2i_t>(set.size());
					}
				}
				os << " │";
				if (grid_i != grid_views.size() - 1) {
					os << "   ";
				}
			}
		}
		print_box_row_sep_strings(O);
	}
}