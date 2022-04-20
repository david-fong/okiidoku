#include <okiidoku/db/serdes.hpp>

#include <iostream>
#include <algorithm>
#include <array>
#include <bitset>
#include <limits>
#include <cassert>

namespace okiidoku::mono::db::serdes {

	template<Order O>
	OKIIDOKU_EXPORT void print_filled(std::ostream& os, const GridConstSpan<O> grid) {
		using T = traits<O>;
		using has_mask_t = T::o2_bits_smol;
		using o2x_smol_t = T::o2x_smol_t;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4i_t = T::o4i_t;

		assert(is_grid_filled<O>(grid));
		unsigned bytes_written = 0;

		// TODO.mid consider a design that uses exact values from not_has masks instead of these
		//  that would mean losing the random access capability in exchange for probably better
		//  average compression. In that case, also try not ignoring diagonal blocks.
		//  ie. replace the std::max(<pop_things>) with cell_not_has.count() or something.
		o2x_smol_t row_pop {0};
		std::array<o2x_smol_t, T::O1> box_pops {0};
		std::array<o2x_smol_t, T::O2> col_pops {0};

		std::array<has_mask_t, T::O2> rows_not_has {0};
		std::array<has_mask_t, T::O2> cols_not_has {0};
		std::array<has_mask_t, T::O2> boxes_not_has {0};

		using buf_t = uint8_t;
		using buf_plus_t = unsigned;
		static_assert(sizeof(buf_plus_t) > sizeof(buf_t));
		static constexpr buf_plus_t buf_ceil = buf_plus_t{std::numeric_limits<buf_t>::max()} + 1u;
		buf_t buf {0};
		buf_plus_t buf_pos {1};

		for (o4i_t i {0}; i < T::O4; ++i) {
			const auto row {static_cast<o2x_t>(i / T::O2)};
			const auto col {static_cast<o2x_t>(i % T::O2)};
			const auto box {static_cast<o2x_t>(rmi_to_box<O>(row, col))};
			if (col == 0) [[unlikely]] { row_pop = 0; }
			if (i % T::O3 == 0) [[unlikely]] { box_pops.fill(0); }
			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue;
			}
			auto& col_pop = col_pops[col];
			auto& box_pop = box_pops[box];
			has_mask_t cell_not_has {rows_not_has[row] & cols_not_has[col] & boxes_not_has[box]};
			o2x_smol_t smol_val {([&]{
				has_mask_t under_val_mask {0};
				under_val_mask.flip();
				under_val_mask >>= T::O2 - grid[i];
				return (cell_not_has & under_val_mask).count();
			})()};
			o2i_t smol_val_remaining {static_cast<o2i_t>(T::O2 - std::max({row_pop, col_pop, box_pop}))};
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
					os.put(buf);
					buf = 0;
					buf_pos = 1;
					++bytes_written;
				}
			}

			++row_pop; ++col_pop; ++box_pop; {
				has_mask_t val_mask {0};
				val_mask.set(static_cast<o2x_t>(grid[i]));
				val_mask.flip();
				rows_not_has[row]  &= val_mask;
				cols_not_has[col]  &= val_mask;
				boxes_not_has[box] &= val_mask;
			}
		}
		if (buf_pos > 1) {
			os.put(buf);
			++bytes_written;
		}
	}


	template<Order O>
	OKIIDOKU_EXPORT void parse_filled(std::istream& is, const GridSpan<O> grid) {
		(void)is; (void)grid; // TODO.high

		// will need some bit parallel deposit for the deserialization
	}


	template<Order O>
	OKIIDOKU_EXPORT void print_puzzle(std::ostream& os, const GridConstSpan<O> grid) {
		(void)os; (void)grid; // TODO.high
	}


	template<Order O>
	OKIIDOKU_EXPORT void parse_puzzle(std::istream& is, const GridSpan<O> grid) {
		(void)is; (void)grid; // TODO.high
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void print_filled<O_>(std::ostream&, GridConstSpan<O_>); \
		template void parse_filled<O_>(std::istream&, GridSpan<O_>); \
		template void print_puzzle<O_>(std::ostream&, GridConstSpan<O_>); \
		template void parse_puzzle<O_>(std::istream&, GridSpan<O_>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}