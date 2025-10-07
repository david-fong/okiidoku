// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/serdes.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream> // include before serdes.hpp (it used iosfwd)
#include <okiidoku/detail/mixed_radix_uint_serdes.hpp>

#include <array>
#include <cstdint>
#include <cstddef>     // size_t
#include <type_traits>

namespace okiidoku::mono { namespace {

	template<Order O, bool is_writer> requires(is_order_compiled(O))
	class SerdesBase {
	protected:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using cands_t = O2BitArr<O>;
		using sym_t = o2x_t; // TODO what about puzzles then? separate bitmap of populated/empty cells?

	private:
		/** hypothetical candidates remaining for future cells
		based on already encountered (printed/parsed) cells. */
		/*      */ cands_t /*   */ row_cands_         {O2BitArr_ones<O>};
		std::array<cands_t, T::O1> h_chute_box_cands_ {[]{ std::array<cands_t, T::O1> _; _.fill(O2BitArr_ones<O>); return _; }()};
		std::array<cands_t, T::O2> cols_cands_        {[]{ std::array<cands_t, T::O2> _; _.fill(O2BitArr_ones<O>); return _; }()};
		o4i_t cell_rmi_ {0u};

		// \internal not protected <- to control member layout.
		std::conditional_t<is_writer,
			::okiidoku::detail::MixedRadixUintWriter<o2is_t, std::uint64_t>,
			::okiidoku::detail::MixedRadixUintReader<o2is_t, std::uint64_t>
		> serdes_ {};

	protected:
		/** \internal I could just make the field protected, but then I can't nit over member layout :P */
		[[nodiscard, gnu::pure]] auto& serdes() & noexcept { return serdes_; }

		/** \pre `!done()` */
		void remove_cand_at_current_rmi(const sym_t cand) noexcept {
			OKIIDOKU_CONTRACT(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT(!done()); cand.check();
			auto& box_cands {h_chute_box_cands_[(rmi()%T::O2) / T::O1]};
			auto& col_cands {cols_cands_[rmi_to_col<O>(rmi())]};
			cands_t::unset3(cand, row_cands_, box_cands, col_cands);
		}

	public:
		SerdesBase() noexcept = default;

		[[nodiscard, gnu::pure]] constexpr
		bool done() const noexcept {
			cell_rmi_.check();
			return cell_rmi_ == T::O4; // or maybe more appropriately/easily- if want to have non-rmi cell-visitation order, `writer_.item_count()`?
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
			OKIIDOKU_CONTRACT(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT(!done());
			auto& box_cands {h_chute_box_cands_[(rmi()%T::O2) / T::O1]};
			auto& col_cands {cols_cands_[rmi_to_col<O>(rmi())]};
			return row_cands_ & col_cands & box_cands;
		}
		/** \pre `!done()` */
		void advance() noexcept {
			OKIIDOKU_CONTRACT(cell_rmi_ < T::O4); OKIIDOKU_CONTRACT(!done());
			++cell_rmi_;
			if (done()) [[unlikely]] { return; }
			if (cell_rmi_ % T::O2 == 0u) [[unlikely]] { row_cands_ = O2BitArr_ones<O>; }
			if (cell_rmi_ % T::O3 == 0u) [[unlikely]] { h_chute_box_cands_.fill(O2BitArr_ones<O>); }
		}
	};


	template<Order O> requires(is_order_compiled(O))
	class Writer final : public SerdesBase<O,true> {
	private:
		using sym_t = typename Ints<O>::o2x_t; // TODO what about puzzles then? separate bitmap of populated/empty cells?
	public:
		// automatically removes printed `sym` as a candidate of future cells.
		void operator()(std::ostream& os, const sym_t sym) {
			OKIIDOKU_CONTRACT(!this->done()); sym.check();
			const auto ctx_cands {this->cands()};
				OKIIDOKU_ASSERT(ctx_cands[sym]); // consistency with precondition that grid follows the one rule.
			const auto cands_count {ctx_cands.count()};
				OKIIDOKU_CONTRACT(cands_count > 0u); // implied by contract (grid_follows_rule)
			const auto compressed_sym {ctx_cands.count_below(sym)};
			if (!this->serdes().accept({.radix{cands_count}, .digit{compressed_sym}})) [[unlikely]] {
				this->serdes().flush(os);
			}
			this->remove_cand_at_current_rmi(sym);
		}
		/** \returns number of bytes written. */
		std::size_t finish(std::ostream& os) {
			OKIIDOKU_CONTRACT(this->done());
			this->serdes().flush(os);
			return this->serdes().byte_count();
		}
	};


	template<Order O> requires(is_order_compiled(O))
	class Reader final : public SerdesBase<O, false> {
	private:
		using sym_t = typename Ints<O>::o2x_t; // TODO what about puzzles then? separate bitmap of populated/empty cells?
	public:
		// automatically removes parsed `sym` as a candidate of future cells.
		[[nodiscard]] sym_t operator()(std::istream& is) {
			OKIIDOKU_CONTRACT(!this->done());
			const auto ctx_cands {this->cands()};
			// The number of possible different values that this cell could be
			// based on the values that have already been encountered.
			const auto cands_count {ctx_cands.count()};
				OKIIDOKU_CONTRACT(cands_count > 0u); // implied by contract (grid_follows_rule)

			const auto compressed_sym {*this->serdes().read(is, cands_count)};
			const auto sym {ctx_cands.get_index_of_nth_set_bit(compressed_sym)};
				OKIIDOKU_ASSERT(ctx_cands[sym]);
			this->remove_cand_at_current_rmi(sym);
			return sym;
		}
		/** \returns number of bytes read. */
		std::size_t finish(std::istream& is) {
			OKIIDOKU_CONTRACT(this->done());
			this->serdes().finish(is);
			return this->serdes().byte_count();
		}
	};
}}

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	std::size_t write_solved(const Grid<O>& grid, std::ostream& os) {
		OKIIDOKU_ASSERT(grid_is_filled(grid));
		OKIIDOKU_ASSERT(grid_follows_rule(grid));
		using T = Ints<O>;

		Writer<O> writer {};
		for (; !writer.done(); writer.advance()) {
			const auto row {writer.rmi() / T::O2};
			const auto col {writer.rmi() % T::O2};
			if ((row/T::O1) == (col/T::O1)) {
				// skip cells in main-diagonal boxes. they can be losslessly reconstructed.
				continue;
			}
			const auto sym {*grid[writer.rmi()]};
			writer(os, sym);
		}
		return writer.finish(os);
	}


	template<Order O> requires(is_order_compiled(O))
	std::size_t read_solved(Grid<O>& grid, std::istream& is) {
		using T = Ints<O>;
		std::size_t bytes_read {[&]{
			// parse out bulk of content (skip what can be reconstructed later):
			Reader<O> reader {};
			for (; !reader.done(); reader.advance()) {
				const auto row {reader.rmi() / T::O2};
				const auto col {reader.rmi() % T::O2};
				if ((row/T::O1) == col/T::O1) [[unlikely]] {
					continue; // skip cells in the main-diagonal boxes
				}
				const auto sym {reader(is)};
				grid[reader.rmi()] = sym;
			}
			return reader.finish(is);
		}()};{
			// reconstruct cells in main-diagonal boxes:
			std::array<std::array<std::array<O2BitArr<O>, T::O1>, T::O1>, T::O1> main_diag_box_cands {};
			for (const auto rmi : T::O4) {
				// get the data needed to losslessly reconstruct:
				const auto row {rmi / T::O2};
				const auto col {rmi % T::O2};
				if ((row/T::O1) == col/T::O1) [[unlikely]] {
					continue; // skip cells that need reconstructing
				}
				for (const auto atom_cell : T::O1) {
					// every cell outside the ones needing constructing can see two of those boxes
					(main_diag_box_cands [row/T::O1] [row%T::O1] [atom_cell]).set(*grid[row,col]);
					(main_diag_box_cands [col/T::O1] [atom_cell] [col%T::O1]).set(*grid[row,col]);
				}
			}
			for (const auto box     : T::O1) {
			for (const auto box_row : T::O1) {
			for (const auto box_col : T::O1) {
				const auto cands {~main_diag_box_cands[box][box_row][box_col]};
				OKIIDOKU_ASSERT(cands.count() == 1u);
				const auto sym {cands.first_set_bit_require_exists()};
				grid[box_cell_to_rmi<O>((box*T::O1)+box, (T::O1*box_row)+box_col)] = sym;
			}}}
		}
		OKIIDOKU_ASSERT(grid_is_filled(grid));
		OKIIDOKU_ASSERT(grid_follows_rule(grid));
		return bytes_read;
	}


	template<Order O> requires(is_order_compiled(O))
	std::size_t write_puzzle(const Grid<O>& grid, std::ostream& os) {
		OKIIDOKU_ASSERT(grid_follows_rule(grid));
		// using T = Ints<O>;
		(void)os; (void)grid; return 0uz; // TODO
	}


	template<Order O> requires(is_order_compiled(O))
	std::size_t read_puzzle(Grid<O>& grid, std::istream& is) {
		// using T = Ints<O>;
		(void)is; (void)grid;

		OKIIDOKU_ASSERT(grid_follows_rule(grid));
		return 0uz; // TODO
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template std::size_t write_solved<(O_)>(const Grid<(O_)>&, std::ostream&); \
		template std::size_t read_solved <(O_)>(      Grid<(O_)>&, std::istream&); \
		template std::size_t write_puzzle<(O_)>(const Grid<(O_)>&, std::ostream&); \
		template std::size_t read_puzzle <(O_)>(      Grid<(O_)>&, std::istream&);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	std::size_t write_solved(const Grid& vis_src, std::ostream& os) {
		switch (vis_src.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::write_solved(vis_src.unchecked_get_mono_exact<(O_)>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	std::size_t read_solved(Grid& vis_sink, std::istream& is) {
		switch (vis_sink.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::read_solved(vis_sink.unchecked_get_mono_exact<(O_)>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	std::size_t write_puzzle(const Grid& vis_src, std::ostream& os) {
		switch (vis_src.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::write_puzzle(vis_src.unchecked_get_mono_exact<(O_)>(), os);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	std::size_t read_puzzle(Grid& vis_sink, std::istream& is) {
		switch (vis_sink.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::read_puzzle(vis_sink.unchecked_get_mono_exact<(O_)>(), is);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}
