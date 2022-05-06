#include <okiidoku/db/serdes.hpp>

#include <okiidoku/house_mask.hpp>

#include <iostream>
#include <algorithm>
#include <array>
#include <limits> // numeric_limits
#include <cassert>

namespace okiidoku::mono::db::serdes {

	namespace detail {
		template<Order O> requires(is_order_compiled(O)) class SerdesHelper;
	}
	template<Order O> requires(is_order_compiled(O))
	class detail::SerdesHelper final {
		using T = traits<O>;
		using cands_t = HouseMask<O>;
		using val_t = typename T::o2x_t;
		using o4i_t = typename T::o4i_t;

		static constexpr unsigned num_buf_bytes {1};
		using buf_t = uint_smolN_t<2*8*num_buf_bytes>; // x2 to prevent overflow
		static_assert((1U<<(2*8*num_buf_bytes)) > (2U*T::O2)); // requirement to handle overflow

	public:
		SerdesHelper() noexcept:
			row_cands {house_mask_ones<O>},
			cell_cands {house_mask_ones<O>}
		{
			h_chute_boxes_cands.fill(house_mask_ones<O>);
			cols_cands.fill(house_mask_ones<O>);
		}
		[[nodiscard, gnu::pure]] o4i_t get_cell_rmi() const noexcept { return cell_rmi; }
		void advance() noexcept;

		// automatically removes val as a candidate of future cells.
		void print_val(std::ostream& os, val_t val) noexcept;
		void print_remaining_buf(std::ostream& os) noexcept;

		// automatically removes val as a candidate of future cells.
		[[nodiscard]] val_t parse_val(std::istream& is) noexcept;

	private:
		void remove_cand_at_current_rmi_(val_t cand) noexcept;

		// hypothetical candidates remaining for future cells
		// based on already encountered (printed/parsed) cells.
		cands_t row_cands;
		std::array<cands_t, T::O1> h_chute_boxes_cands;
		std::array<cands_t, T::O2> cols_cands;

		// hypothetical candidates of cell to print/parse.
		cands_t cell_cands;

		o4i_t cell_rmi {0};

		// current small buffer of data to print/parse.
		static constexpr buf_t buf_end {buf_t{8*num_buf_bytes}};
		buf_t buf {0};
		buf_t buf_pos {1};
	};


	template<Order O> requires(is_order_compiled(O))
	void detail::SerdesHelper<O>::advance() noexcept {
		++cell_rmi;
		if (cell_rmi % T::O2 == 0) [[unlikely]] { row_cands = house_mask_ones<O>; }
		if (cell_rmi % T::O3 == 0) [[unlikely]] { h_chute_boxes_cands.fill(house_mask_ones<O>); }

		auto& box_cands {h_chute_boxes_cands[cell_rmi / T::O3]};
		auto& col_cands {cols_cands[cell_rmi / T::O2]};
		cell_cands = row_cands & col_cands & box_cands;
	}


	template<Order O> requires(is_order_compiled(O))
	void detail::SerdesHelper<O>::print_val(std::ostream& os, const typename detail::SerdesHelper<O>::val_t val) noexcept {
		// The number of possible different values that this cell could be
		// based on the values that have already been encountered.
		auto smol_val_buf_remaining {cell_cands.count()};
		assert(smol_val_buf_remaining > 0); // implied by contract (grid_follows_rule)

		// Some slightly-weird-looking logic stems from the fact that it is
		// a "null" action to try to print something that can only take on one
		// value (as in- the buffer will be unchanged). Just keep that in mind.
		auto smol_val_buf {static_cast<val_t>(cell_cands.count_bits_below(val))};
		assert(smol_val_buf < smol_val_buf_remaining);
		while (smol_val_buf_remaining > 1) {
			buf += static_cast<buf_t>(buf_pos * smol_val_buf); // should never overflow
			// const auto buf_remaining {(buf_pos == 1) ? buf_end : static_cast<buf_t>(buf_end - buf_pos)};
			{
				const auto use_factor {static_cast<buf_t>(smol_val_buf_remaining)};
				assert(buf_pos != 0 && buf_pos < buf_end);
				buf_pos *= use_factor;
				smol_val_buf /= static_cast<val_t>(use_factor);
				smol_val_buf_remaining /= static_cast<typename T::o2i_t>(use_factor);
			}
			if (buf_pos >= buf_end) { // TODO.asap should this be a while loop?
				static_assert(num_buf_bytes == 1); // otherwise the below needs to change.
				os.put(static_cast<char>(buf));
				buf >>= 8 * num_buf_bytes;
				buf_pos = static_cast<buf_t>((buf_pos % buf_end) + 1);
			}
		}
		remove_cand_at_current_rmi_(val);
	}


	template<Order O> requires(is_order_compiled(O))
	void detail::SerdesHelper<O>::print_remaining_buf(std::ostream& os) noexcept {
		if (buf_pos > 1) {
			os.put(static_cast<char>(buf));
		}
		// TODO should there be some mechanism to disable further printing? or just write a contract?
	}


	template<Order O> requires(is_order_compiled(O))
	typename detail::SerdesHelper<O>::val_t detail::SerdesHelper<O>::parse_val(std::istream& is) noexcept {
		// The number of possible different values that this cell could be
		// based on the values that have already been encountered.
		auto smol_val_buf_remaining {cell_cands.count()};
		assert(smol_val_buf_remaining > 0); // implied by contract (grid_follows_rule)
		(void)smol_val_buf_remaining; (void)is;

		val_t smol_val_buf {0};
		// TODO.asap

		assert(smol_val_buf < smol_val_buf_remaining);
		const auto val {cell_cands.get_index_of_nth_set_bit(smol_val_buf)};
		remove_cand_at_current_rmi_(val);
		return val;
	}


	template<Order O> requires(is_order_compiled(O))
	void detail::SerdesHelper<O>::remove_cand_at_current_rmi_(const typename SerdesHelper<O>::val_t cand) noexcept {
		auto& box_cands {h_chute_boxes_cands[cell_rmi / T::O3]};
		auto& col_cands {cols_cands[cell_rmi / T::O2]};
		cands_t::unset3(cand, row_cands, box_cands, col_cands);
	}


	template<Order O>
	void print_filled(std::ostream& os, const Grid<O>& grid) noexcept {
		assert(grid_is_filled<O>(grid));
		assert(grid_follows_rule(grid));
		using T = traits<O>;

		detail::SerdesHelper<O> helper {};
		for (; helper.get_cell_rmi() < T::O4; helper.advance()) {
			const auto row {helper.get_cell_rmi() / T::O2};
			const auto col {helper.get_cell_rmi() % T::O2};
			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue; // skip cells in the anti-diagonal boxes
			}
			using val_t = typename traits<O>::o2x_t;
			const auto val {static_cast<val_t>(grid.at_row_major(helper.get_cell_rmi()))};
			helper.print_val(os, val);
		}
		helper.print_remaining_buf(os);
	}


	template<Order O>
	void parse_filled(std::istream& is, Grid<O>& grid) noexcept {
		using T = traits<O>;

		detail::SerdesHelper<O> helper {};
		for (; helper.get_cell_rmi() < T::O4; helper.advance()) {
			const auto row {helper.get_cell_rmi() / T::O2};
			const auto col {helper.get_cell_rmi() % T::O2};
			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue; // skip cells in the anti-diagonal boxes
			}
			const auto val {helper.parse_val(is)};
			grid.at_row_major(helper.get_cell_rmi()) = static_cast<grid_val_t<O>>(val);
		}
		// TODO.asap infer cells in anti-diagonal boxes.

		assert(grid_is_filled(grid));
		assert(grid_follows_rule(grid));
	}


	template<Order O>
	void print_puzzle(std::ostream& os, const Grid<O>& grid) noexcept {
		assert(grid_follows_rule(grid));
		// using T = traits<O>;
		(void)os; (void)grid;
	}


	template<Order O>
	void parse_puzzle(std::istream& is, Grid<O>& grid) noexcept {
		// using T = traits<O>;
		(void)is; (void)grid;

		assert(grid_follows_rule(grid));
		assert(grid_follows_rule(grid));
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void print_filled<O_>(std::ostream&, const Grid<O_>&) noexcept; \
		template void parse_filled<O_>(std::istream&, Grid<O_>&) noexcept; \
		template void print_puzzle<O_>(std::ostream&, const Grid<O_>&) noexcept; \
		template void parse_puzzle<O_>(std::istream&, Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::db::serdes {

	void print_filled(std::ostream& os, const Grid& vis_src) noexcept {
		return std::visit([&](auto& mono_src) {
			return mono::db::serdes::print_filled(os, mono_src);
		}, vis_src.get_mono_variant());
	}

	void parse_filled(std::istream& is, Grid& vis_sink) noexcept {
		return std::visit([&](auto& mono_sink) {
			return mono::db::serdes::parse_filled(is, mono_sink);
		}, vis_sink.get_mono_variant());
	}

	void print_puzzle(std::ostream& os, const Grid& vis_src) noexcept {
		return std::visit([&](auto& mono_src) {
			return mono::db::serdes::print_puzzle(os, mono_src);
		}, vis_src.get_mono_variant());
	}

	void parse_puzzle(std::istream& is, Grid& vis_sink) noexcept {
		return std::visit([&](auto& mono_sink) {
			return mono::db::serdes::parse_puzzle(is, mono_sink);
		}, vis_sink.get_mono_variant());
	}
}