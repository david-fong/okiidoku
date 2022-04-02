#include <okiidoku/print_2d.hpp>
#include <okiidoku/emoji.hpp>
#include <okiidoku_util/str.hpp>
#include <okiidoku/rng.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>

namespace okiidoku {

	// current implementation is pretty simple (dumb?)
	std::vector<size_t> make_random_emoji_set(const unsigned O) {
		const unsigned O2 = O*O;
		const auto& prefs = emoji::top_set_preferences;
		std::vector<size_t> shuffled_sets(emoji::sets.size());
		std::iota(shuffled_sets.begin(), shuffled_sets.end(), 0);
		{
			std::lock_guard rng_guard_ {shared_mt_rng_mutex_};
			for (
				size_t b {0}, e_i_ {0}, e {prefs[e_i_]};
				b != prefs.back();
				b = e, ++e_i_, e = (e_i_ == prefs.size() ? emoji::sets.size() : prefs[e_i_])
			) {
				std::shuffle(shuffled_sets.begin() + b, shuffled_sets.begin() + e, shared_mt_rng_);
			}
		}
		// first try to find a single set large enough:
		for (
			size_t b {0}, e_i_ {0}, e {prefs[e_i_]};
			e != prefs.back(); // unlike before, don't use anything not in top prefs list here.
			b = e, ++e_i_, e = prefs[e_i_]
		) {
			for (size_t i {b}; i < e; ++i) {
				if (emoji::sets.at(shuffled_sets.at(i)).entries.size() >= O2) {
					return {shuffled_sets.at(i)};
			}	}
		}
		// otherwise just return everything:
		return shuffled_sets;
	}


	void print_2d(std::ostream& os, const unsigned O, const std::span<const print_2d_grid_view> grid_views) {
		namespace str = okiidoku::util::str;
		using o2i_t = std::uint16_t;

		const auto print_blk_row_sep_string_ = [&os, O](const unsigned border_i) -> void {
			#define M_NOOK(NOOK_T, NOOK_C, NOOK_B) \
			if      (border_i == 0) [[unlikely]] { os << NOOK_T; } \
			else if (border_i == O) [[unlikely]] { os << NOOK_B; } \
			else                    { os << NOOK_C; }
			M_NOOK(" ┌", " ├", " └")
			for (unsigned blk_col {0}; blk_col < O; ++blk_col) {
				for (unsigned i {0}; i < 1u + (2u * O); ++i) {
					os << "─";
				}
				if (blk_col < O - 1u) { M_NOOK("┬", "┼", "┴") }
			}
			M_NOOK("┐", "┤", "┘")
			#undef M_NOOK
		};

		auto print_blk_row_sep_strings = [&](const unsigned border_i) mutable {
			os << '\n';
			print_blk_row_sep_string_(border_i);
			for (unsigned i {1}; i < grid_views.size(); ++i) {
				os << "   ";
				print_blk_row_sep_string_(border_i);
			}
		};
		const auto emoji_sets = make_random_emoji_set(O);

		os << str::dim.on;
		for (o2i_t row {0}; row < O*O; ++row) {
			if (row % O == 0) {
				print_blk_row_sep_strings(row / O);
			}
			os << '\n';
			for (unsigned grid_i {0}; grid_i < grid_views.size(); ++grid_i) {
				for (o2i_t col {0}; col < O*O; ++col) {
					if ((col % O) == 0) { os << str::dim.on << " │" << str::dim.off; }

					auto val = size_t{grid_views[grid_i](row * O*O + col)};
					if (val == O*O) {
						os << "  ";
						continue;
					}
					for (const auto emoji_set_index : emoji_sets) {
						const auto& set = emoji::sets.at(emoji_set_index).entries;
						if (val < set.size()) {
							os << set.at(val);
							break;
						} else {
							val -= set.size();
					}	}
				}
				os << str::dim.on << " │";
				if (grid_i != grid_views.size() - 1) {
					os << "   ";
				}
			}
		}
		print_blk_row_sep_strings(O);
		os << str::dim.off;
	}
}