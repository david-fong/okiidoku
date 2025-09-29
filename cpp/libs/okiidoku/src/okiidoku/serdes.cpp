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
#include <climits> // CHAR_BIT

namespace okiidoku::mono { namespace {

	template<Order O> requires(is_order_compiled(O))
	class SerdesHelper {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		// using int_writer_t = ::okiidoku::detail::MixedRadixUintWriter<T::o2is_t>;
		using cands_t = O2BitArr<O>;
		using sym_t = T::o2x_t; // TODO what about puzzles then? separate bitmap of populated/empty cells?

		static constexpr unsigned num_buf_bytes {1u};
		using buf_t = ::okiidoku::detail::uint_small_for_width_t<2u*CHAR_BIT*num_buf_bytes>; // x2 to prevent overflow
		static_assert((1uLL<<(2u*CHAR_BIT*num_buf_bytes)) > (2u*T::O2)); // requirement to handle overflow

	public:
		constexpr SerdesHelper() noexcept = default;

		[[nodiscard, gnu::pure]] constexpr
		bool done() const noexcept {
			return cell_rmi_ == T::O4;
		}

		/** get current row-major index of a grid to print/parse.
		\pre `!done()` */
		[[nodiscard, gnu::pure]] constexpr
		o4x_t rmi() const noexcept {
			return *cell_rmi_;
		}

		/** get symbol candidates at current `rmi()`.
		\pre `!done()` */
		[[nodiscard, gnu::pure]]
		auto cands() const noexcept {
			OKIIDOKU_CONTRACT_USE(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT_USE(!done());
			auto& box_cands {h_chute_box_cands_[rmi() / T::O1]};
			auto& col_cands {cols_cands_[rmi_to_col<O>(rmi())]};
			return row_cands_ & col_cands & box_cands;
		}

		/** \pre `!done()` */
		void advance() noexcept {
			OKIIDOKU_CONTRACT_USE(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT_USE(!done());
			++cell_rmi_;
			if (done()) { return; }
			if (cell_rmi_ % T::O2 == 0u) [[unlikely]] { row_cands_ = O2BitArr_ones<O>; }
			if (cell_rmi_ % T::O3 == 0u) [[unlikely]] { h_chute_box_cands_.fill(O2BitArr_ones<O>); }
		}

	private:
		/** \pre `!done()` */
		void remove_cand_at_current_rmi(const sym_t cand) noexcept {
			OKIIDOKU_CONTRACT_USE(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT_USE(!done());
			auto& box_cands {h_chute_box_cands_[cell_rmi_ / T::O3]};
			auto& col_cands {cols_cands_[cell_rmi_ / T::O2]};
			cands_t::unset3(cand, row_cands_, box_cands, col_cands);
		}

		/** hypothetical candidates remaining for future cells
		based on already encountered (printed/parsed) cells. */
		/*      */ cands_t /*   */ row_cands_         {O2BitArr_ones<O>};
		std::array<cands_t, T::O1> h_chute_box_cands_ {[]{ std::array<cands_t, T::O1> _; _.fill(O2BitArr_ones<O>); return _; }()};
		std::array<cands_t, T::O2> cols_cands_        {[]{ std::array<cands_t, T::O2> _; _.fill(O2BitArr_ones<O>); return _; }()};

		o4i_t cell_rmi_ {0u};

		// current small buffer of data to print/parse.
		static constexpr buf_t buf_end {CHAR_BIT*num_buf_bytes};
		buf_t buf_ {0u};
		buf_t buf_pos_ {1u};

	public:
		// automatically removes printed `sym` as a candidate of future cells.
		void print_sym(std::ostream& os, sym_t sym) noexcept {
			const auto ctx_cands {cands()};
			OKIIDOKU_CONTRACT_ASSERT(ctx_cands[sym]); // consistency with precondition that grid follows the one rule.
			auto smol_sym_buf_remaining {ctx_cands.count()}; // num possible remaining symbols here
			OKIIDOKU_CONTRACT_USE(smol_sym_buf_remaining > 0u); // implied by contract (grid_follows_rule)

			// Some slightly-weird-looking logic stems from the fact that it is
			// a "null" action to try to print something that can only take on one
			// value (as in- the buffer will be unchanged). Just keep that in mind.
			auto smol_sym_buf {ctx_cands.count_below(sym)};
			OKIIDOKU_CONTRACT_USE(smol_sym_buf < smol_sym_buf_remaining);
			while (smol_sym_buf_remaining > 1u) {
				buf_ += static_cast<buf_t>(buf_pos_ * smol_sym_buf); // should never overflow
				// const auto buf_remaining {(buf_pos_ == 1) ? buf_end : static_cast<buf_t>(buf_end - buf_pos_)};
				{
					const auto use_factor {static_cast<buf_t>(smol_sym_buf_remaining)};
					OKIIDOKU_CONTRACT_USE(buf_pos_ != 0u && buf_pos_ < buf_end);
					buf_pos_ *= use_factor;
					smol_sym_buf /= static_cast<sym_t>(use_factor);
					smol_sym_buf_remaining /= o2i_t{use_factor};
				}
				if (buf_pos_ >= buf_end) { // TODO.asap should this be a while loop?
					static_assert(num_buf_bytes == 1u); // otherwise the below needs to change.
					os.put(static_cast<char>(buf_));
					buf_ >>= CHAR_BIT * num_buf_bytes;
					buf_pos_ = static_cast<buf_t>((buf_pos_ % buf_end) + 1u);
				}
			}
			remove_cand_at_current_rmi(sym);
		}
		void print_remaining_buf(std::ostream& os) noexcept {
			if (buf_pos_ > 1u) {
				os.put(static_cast<char>(buf_));
			}
			// TODO should there be some mechanism to disable further printing? or just write a contract?
		}

		// automatically removes parsed `sym` as a candidate of future cells.
		[[nodiscard]] sym_t parse_sym(std::istream& is) noexcept {
			const auto ctx_cands {cands()};
			// The number of possible different values that this cell could be
			// based on the values that have already been encountered.
			auto smol_sym_buf_remaining {ctx_cands.count()};
			OKIIDOKU_CONTRACT_USE(smol_sym_buf_remaining > 0u); // implied by contract (grid_follows_rule)
			(void)smol_sym_buf_remaining; (void)is;

			sym_t smol_sym_buf {0u};
			// TODO

			OKIIDOKU_CONTRACT_USE(smol_sym_buf < smol_sym_buf_remaining);
			const auto sym {ctx_cands.get_index_of_nth_set_bit(smol_sym_buf)};
			remove_cand_at_current_rmi(sym);
			return sym;
		}
	};
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void write_solution_grid_to_stream(const Grid<O>& grid, std::ostream& os) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled<O>(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
		using T = Ints<O>;

		SerdesHelper<O> helper {};
		for (; !helper.done(); helper.advance()) {
			const auto row {helper.rmi() / T::O2};
			const auto col {helper.rmi() % T::O2};
			if ((row/T::O1) == (col/T::O1)) {
				// skip cells in main-diagonal boxes. they can be losslessly reconstructed.
				continue;
			}
			const auto sym {*grid[helper.rmi()]};
			helper.print_sym(os, sym);
		}
		helper.print_remaining_buf(os);
	}


	template<Order O> requires(is_order_compiled(O))
	void parse_solution_grid_from_stream(Grid<O>& grid, std::istream& is) noexcept {
		using T = Ints<O>;
		{
			// parse out bulk of content (skip what can be reconstructed later):
			SerdesHelper<O> helper {};
			for (; !helper.done(); helper.advance()) {
				const auto row {helper.rmi() / T::O2};
				const auto col {helper.rmi() % T::O2};
				if ((row/T::O1) == col/T::O1) {
					continue; // skip cells in the main-diagonal boxes
				}
				const auto sym {helper.parse_sym(is)};
				grid[helper.rmi()] = sym;
			}
		}{
			// reconstruct cells in main-diagonal boxes:
			std::array<std::array<std::array<O2BitArr<O>, T::O1>, T::O1>, T::O1> main_diag_box_cands {};
			for (const auto rmi : T::O4) {
				// get the data needed to losslessly reconstruct:
				const auto row {rmi / T::O2};
				const auto col {rmi % T::O2};
				if ((row/T::O1) == col/T::O1) {
					continue; // skip cells that need reconstructing
				}
				for (const auto atom_cell : T::O1) {
					(main_diag_box_cands [row/T::O1] [row%T::O1] [atom_cell]).set(*grid[row,col]);
					(main_diag_box_cands [col/T::O1] [atom_cell] [col%T::O1]).set(*grid[row,col]);
				}
			}
			for (const auto box     : T::O1) {
			for (const auto box_row : T::O1) {
			for (const auto box_col : T::O1) {
				const auto taken {~main_diag_box_cands[box][box_row][box_col]};
				OKIIDOKU_CONTRACT_ASSERT(taken.count() == T::O1.prev());
				const auto sym {taken.get_index_of_nth_set_bit(0u)};
				grid[box_cell_to_rmi<O>(box, (T::O1*box_row)+box_col)] = sym;
			}}}
		}
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
		switch (vis_src.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::write_solution_grid_to_stream(vis_src.unchecked_get_mono_exact<O_>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void parse_solution_grid_from_stream(Grid& vis_sink, std::istream& is) noexcept {
		switch (vis_sink.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::parse_solution_grid_from_stream(vis_sink.unchecked_get_mono_exact<O_>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void print_puzzle_grid_to_stream(const Grid& vis_src, std::ostream& os) noexcept {
		switch (vis_src.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::print_puzzle_grid_to_stream(vis_src.unchecked_get_mono_exact<O_>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void parse_puzzle_grid_from_stream(Grid& vis_sink, std::istream& is) noexcept {
		switch (vis_sink.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::parse_puzzle_grid_from_stream(vis_sink.unchecked_get_mono_exact<O_>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}
