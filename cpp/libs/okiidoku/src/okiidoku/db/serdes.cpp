#include <okiidoku/db/serdes.hpp>

#include <okiidoku/house_mask.hpp>

#include <iostream>
#include <algorithm>
#include <array>
#include <limits> // numeric_limits
#include <cassert>

namespace okiidoku::mono::db::serdes {

	template<Order O>
	void print_filled(std::ostream& os, const Grid<O>& grid) {
		using T = traits<O>;
		using cands_t = HouseMask<O>;
		using o2x_smol_t = typename T::o2x_smol_t;
		using o2x_t = typename T::o2x_t;
		using o2i_t = typename T::o2i_t;
		using o4i_t = typename T::o4i_t;

		assert(grid_is_filled<O>(grid));
		unsigned bytes_written {0};

		cands_t row_cands;
		std::array<cands_t, T::O1> boxes_cands;
		std::array<cands_t, T::O2> cols_cands; cols_cands.fill(cands_t::ones);

		using buf_t = uint8_t;
		using buf_plus_t = unsigned;
		static_assert(sizeof(buf_plus_t) > sizeof(buf_t));
		static constexpr buf_plus_t buf_ceil = buf_plus_t{std::numeric_limits<buf_t>::max()} + 1u;
		buf_t buf {0};
		buf_plus_t buf_pos {1};

		for (o4i_t i {0}; i < T::O4; ++i) {
			const auto row {static_cast<o2x_t>(i / T::O2)};
			const auto col {static_cast<o2x_t>(i % T::O2)};
			const auto val = static_cast<o2x_t>(grid.at_row_major(i));

			if (col == 0) [[unlikely]] { row_cands = cands_t::ones; }
			if (i % T::O3 == 0) [[unlikely]] { boxes_cands.fill(cands_t::ones); }

			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue; // skip cells in the anti-diagonal boxes
				// Note: this _must_ go after the above resets of row_cands and boxes_cands.
			}

			auto& box_cands {boxes_cands[col / T::O1]};
			auto& col_cands {cols_cands[col]};
			const auto cell_cands {row_cands | col_cands | box_cands};
			auto smol_val {static_cast<o2x_smol_t>(cell_cands.count_bits_below(val))};
			auto smol_val_remaining {cell_cands.count()};
			assert(smol_val_remaining > 0);
			assert(smol_val < smol_val_remaining);
			while (smol_val_remaining > 1) {
				buf += static_cast<buf_t>(smol_val * buf_pos);
				{
					assert(buf_pos < buf_ceil);
					const auto use_factor {1 + std::min(buf_ceil - buf_pos, static_cast<buf_plus_t>(smol_val_remaining))};
					buf_pos *= use_factor;
					smol_val /= static_cast<o2x_smol_t>(use_factor);
					smol_val_remaining /= static_cast<o2i_t>(use_factor);
					assert(buf_pos <= buf_ceil);
				}
				if (buf_pos == buf_ceil) {
					os.put(static_cast<char>(buf));
					buf = 0;
					buf_pos = 1;
					++bytes_written;
				}
			}
			cands_t::unset3(val, row_cands, col_cands, box_cands);
		}
		if (buf_pos > 1) {
			os.put(static_cast<char>(buf));
			++bytes_written;
		}
	}


	template<Order O>
	void parse_filled(std::istream& is, Grid<O>& grid) {
		(void)is; (void)grid; // TODO.high

		// will need some bit parallel deposit for the deserialization
	}


	template<Order O>
	void print_puzzle(std::ostream& os, const Grid<O>& grid) {
		(void)os; (void)grid; // TODO.high
	}


	template<Order O>
	void parse_puzzle(std::istream& is, Grid<O>& grid) {
		(void)is; (void)grid; // TODO.high
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void print_filled<O_>(std::ostream&, const Grid<O_>&); \
		template void parse_filled<O_>(std::istream&, Grid<O_>&); \
		template void print_puzzle<O_>(std::ostream&, const Grid<O_>&); \
		template void parse_puzzle<O_>(std::istream&, Grid<O_>&);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::db::serdes {

	void print_filled(std::ostream& os, const Grid& vis_src) {
		return std::visit([&](auto& mono_src) {
			return mono::db::serdes::print_filled(os, mono_src);
		}, vis_src.get_mono_variant());
	}

	void parse_filled(std::istream& is, Grid& vis_sink) {
		return std::visit([&](auto& mono_sink) {
			return mono::db::serdes::parse_filled(is, mono_sink);
		}, vis_sink.get_mono_variant());
	}

	void print_puzzle(std::ostream& os, const Grid& vis_src) {
		return std::visit([&](auto& mono_src) {
			return mono::db::serdes::print_puzzle(os, mono_src);
		}, vis_src.get_mono_variant());
	}

	void parse_puzzle(std::istream& is, Grid& vis_sink) {
		return std::visit([&](auto& mono_sink) {
			return mono::db::serdes::parse_puzzle(is, mono_sink);
		}, vis_sink.get_mono_variant());
	}
}