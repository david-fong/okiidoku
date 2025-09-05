// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/serdes.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>
// #include <algorithm>
#include <array>
// #include <limits> // numeric_limits

namespace okiidoku::mono { namespace {

	template<Order O> requires(is_order_compiled(O))
	class SerdesHelper {
	private:
		using T = Ints<O>;
		using cands_t = O2BitArr<O>;
		using val_t = T::o2x_t;
		using o4i_t = T::o4i_t;

		static constexpr unsigned num_buf_bytes {1};
		using buf_t = detail::uint_small_for_width_t<2*8*num_buf_bytes>; // x2 to prevent overflow
		static_assert((1u<<(2u*8u*num_buf_bytes)) > (2u*T::O2)); // requirement to handle overflow

	public:
		SerdesHelper() noexcept:
			row_cands_ {O2BitArr_ones<O>},
			cell_cands_ {O2BitArr_ones<O>}
		{
			h_chute_boxes_cands_.fill(O2BitArr_ones<O>);
			cols_cands_.fill(O2BitArr_ones<O>);
		}
		[[nodiscard, gnu::pure]] o4i_t get_cell_rmi() const noexcept { return cell_rmi_; }
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
		cands_t row_cands_;
		std::array<cands_t, T::O1> h_chute_boxes_cands_;
		std::array<cands_t, T::O2> cols_cands_;

		// hypothetical candidates of cell to print/parse.
		cands_t cell_cands_;

		o4i_t cell_rmi_ {0};

		// current small buffer of data to print/parse.
		static constexpr buf_t buf_end {buf_t{8*num_buf_bytes}};
		buf_t buf_ {0};
		buf_t buf_pos_ {1};
	};


	template<Order O> requires(is_order_compiled(O))
	void SerdesHelper<O>::advance() noexcept {
		++cell_rmi_;
		if (cell_rmi_ % T::O2 == 0) [[unlikely]] { row_cands_ = O2BitArr_ones<O>; }
		if (cell_rmi_ % T::O3 == 0) [[unlikely]] { h_chute_boxes_cands_.fill(O2BitArr_ones<O>); }

		auto& box_cands {h_chute_boxes_cands_[cell_rmi_ / T::O3]};
		auto& col_cands {cols_cands_[cell_rmi_ / T::O2]};
		cell_cands_ = row_cands_ & col_cands & box_cands;
	}


	template<Order O> requires(is_order_compiled(O))
	void SerdesHelper<O>::print_val(std::ostream& os, const typename SerdesHelper<O>::val_t val) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(cell_cands_[val]);
		// The number of possible different values that this cell could be
		// based on the values that have already been encountered.
		auto smol_val_buf_remaining {cell_cands_.count()};
		OKIIDOKU_CONTRACT_USE(smol_val_buf_remaining > 0); // implied by contract (grid_follows_rule)

		// Some slightly-weird-looking logic stems from the fact that it is
		// a "null" action to try to print something that can only take on one
		// value (as in- the buffer will be unchanged). Just keep that in mind.
		auto smol_val_buf {static_cast<val_t>(cell_cands_.count_below(val))};
		OKIIDOKU_CONTRACT_USE(smol_val_buf < smol_val_buf_remaining);
		while (smol_val_buf_remaining > 1) {
			buf_ += static_cast<buf_t>(buf_pos_ * smol_val_buf); // should never overflow
			// const auto buf_remaining {(buf_pos_ == 1) ? buf_end : static_cast<buf_t>(buf_end - buf_pos_)};
			{
				const auto use_factor {static_cast<buf_t>(smol_val_buf_remaining)};
				OKIIDOKU_CONTRACT_USE(buf_pos_ != 0 && buf_pos_ < buf_end);
				buf_pos_ *= use_factor;
				smol_val_buf /= static_cast<val_t>(use_factor);
				smol_val_buf_remaining /= static_cast<Ints<O>::o2i_t>(use_factor);
			}
			if (buf_pos_ >= buf_end) { // TODO.asap should this be a while loop?
				static_assert(num_buf_bytes == 1); // otherwise the below needs to change.
				os.put(static_cast<char>(buf_));
				buf_ >>= 8 * num_buf_bytes;
				buf_pos_ = static_cast<buf_t>((buf_pos_ % buf_end) + 1);
			}
		}
		remove_cand_at_current_rmi_(val);
	}


	template<Order O> requires(is_order_compiled(O))
	void SerdesHelper<O>::print_remaining_buf(std::ostream& os) noexcept {
		if (buf_pos_ > 1) {
			os.put(static_cast<char>(buf_));
		}
		// TODO should there be some mechanism to disable further printing? or just write a contract?
	}


	template<Order O> requires(is_order_compiled(O))
	typename SerdesHelper<O>::val_t SerdesHelper<O>::parse_val(std::istream& is) noexcept {
		// The number of possible different values that this cell could be
		// based on the values that have already been encountered.
		auto smol_val_buf_remaining {cell_cands_.count()};
		OKIIDOKU_CONTRACT_USE(smol_val_buf_remaining > 0); // implied by contract (grid_follows_rule)
		(void)smol_val_buf_remaining; (void)is;

		val_t smol_val_buf {0};
		// TODO

		OKIIDOKU_CONTRACT_USE(smol_val_buf < smol_val_buf_remaining);
		const auto val {cell_cands_.get_index_of_nth_set_bit(smol_val_buf)};
		remove_cand_at_current_rmi_(val);
		return val;
	}


	template<Order O> requires(is_order_compiled(O))
	void SerdesHelper<O>::remove_cand_at_current_rmi_(const typename SerdesHelper<O>::val_t cand) noexcept {
		auto& box_cands {h_chute_boxes_cands_[cell_rmi_ / T::O3]};
		auto& col_cands {cols_cands_[cell_rmi_ / T::O2]};
		cands_t::unset3(cand, row_cands_, box_cands, col_cands);
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void write_solution_grid_to_stream(const Grid<O>& grid, std::ostream& os) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled<O>(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
		using T = Ints<O>;

		SerdesHelper<O> helper {};
		for (; helper.get_cell_rmi() < T::O4; helper.advance()) {
			const auto row {helper.get_cell_rmi() / T::O2};
			const auto col {helper.get_cell_rmi() % T::O2};
			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue; // skip cells in the anti-diagonal boxes
			}
			using val_t = T::o2x_t;
			const auto val {static_cast<val_t>(grid.at_rmi(helper.get_cell_rmi()))};
			helper.print_val(os, val);
		}
		helper.print_remaining_buf(os);
	}


	template<Order O> requires(is_order_compiled(O))
	void parse_solution_grid_from_stream(Grid<O>& grid, std::istream& is) noexcept {
		using T = Ints<O>;

		SerdesHelper<O> helper {};
		for (; helper.get_cell_rmi() < T::O4; helper.advance()) {
			const auto row {helper.get_cell_rmi() / T::O2};
			const auto col {helper.get_cell_rmi() % T::O2};
			if ((T::O1-1-row)/T::O1 == col/T::O1) {
				continue; // skip cells in the anti-diagonal boxes
			}
			const auto val {helper.parse_val(is)};
			grid.at_rmi(helper.get_cell_rmi()) = static_cast<grid_val_t<O>>(val);
		}
		// TODO infer cells in anti-diagonal boxes.

		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
	}


	template<Order O> requires(is_order_compiled(O))
	void print_puzzle_grid_to_stream(const Grid<O>& grid, std::ostream& os) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
		// using T = Ints<O>;
		(void)os; (void)grid;
	}


	template<Order O> requires(is_order_compiled(O))
	void parse_puzzle_grid_from_stream(Grid<O>& grid, std::istream& is) noexcept {
		// using T = Ints<O>;
		(void)is; (void)grid;

		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template void write_solution_grid_to_stream  <O_>(const Grid<O_>&, std::ostream&) noexcept; \
		template void parse_solution_grid_from_stream<O_>(      Grid<O_>&, std::istream&) noexcept; \
		template void print_puzzle_grid_to_stream    <O_>(const Grid<O_>&, std::ostream&) noexcept; \
		template void parse_puzzle_grid_from_stream  <O_>(      Grid<O_>&, std::istream&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	void write_solution_grid_to_stream(const Grid& vis_src, std::ostream& os) noexcept {
		switch (vis_src.get_mono_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::write_solution_grid_to_stream(vis_src.unchecked_get_mono_exact<O_>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void parse_solution_grid_from_stream(Grid& vis_sink, std::istream& is) noexcept {
		switch (vis_sink.get_mono_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::parse_solution_grid_from_stream(vis_sink.unchecked_get_mono_exact<O_>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void print_puzzle_grid_to_stream(const Grid& vis_src, std::ostream& os) noexcept {
		switch (vis_src.get_mono_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::print_puzzle_grid_to_stream(vis_src.unchecked_get_mono_exact<O_>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void parse_puzzle_grid_from_stream(Grid& vis_sink, std::istream& is) noexcept {
		switch (vis_sink.get_mono_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::parse_puzzle_grid_from_stream(vis_sink.unchecked_get_mono_exact<O_>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}
