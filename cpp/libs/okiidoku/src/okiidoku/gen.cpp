#include <okiidoku/gen.hpp>

#include <random>    // minstd_rand
#include <algorithm> // swap, copy, shuffle, count
#include <numeric>   // iota
#include <array>
#include <cassert>

namespace okiidoku::mono { namespace {

	// Note: using a scoped rng to avoid holding the shared_rng mutex
	// during the long-running parts of this function body.
	using rng_t = std::minstd_rand; // other good LCG parameters: https://arxiv.org/pdf/2001.05304v3.pdf


	template<Order O> requires(is_order_compiled(O))
	struct CountSymsInInvalidBox final {
		using T = Ints<O>;
		using V = typename T::o1i_t;
		using o3i_t = typename T::o3i_t;
		[[nodiscard]] o3i_t count_num_missing_syms() const noexcept { return static_cast<o3i_t>(std::ranges::count(store_, V{0})); }
		template<class T_box, class T_sym> requires(Any_o1x<O, T_box> && Any_o2x<O, T_sym>)
		[[nodiscard]] const V& box_count_sym(const T_box box, const T_sym sym) const noexcept { return store_[(T::O1*sym)+box]; }
		template<class T_box, class T_sym> requires(Any_o1x<O, T_box> && Any_o2x<O, T_sym>)
		[[nodiscard]]       V& box_count_sym(const T_box box, const T_sym sym)       noexcept { return store_[(T::O1*sym)+box]; }
	private:
		// rows for each symbol, entry-in-row for each box.
		std::array<V, T::O3> store_ {0};
	};

	template<Order O> requires(is_order_compiled(O))
	void make_boxes_valid(Grid<O>& grid, const typename Ints<O>::o2i_t h_chute, rng_t& rng_) noexcept {
		using T = Ints<O>;
		using o1x_t = typename T::o1x_t;
		using o2x_t = typename T::o2x_t;
		using o2i_t = typename T::o2i_t;
		using o3i_t = typename T::o3i_t;

		// unsigned long long op_count {0};
		CountSymsInInvalidBox<O> boxes_has {};
		for (o2i_t row {h_chute}; row < h_chute+T::O1; ++row) {
		for (o2i_t col {0}; col < T::O2; ++col) {
			++(boxes_has.box_count_sym(static_cast<o1x_t>(col/T::O1), grid.at(row,col)));
		}}
		o3i_t num_missing_syms {boxes_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {static_cast<o2x_t>((rng_() - rng_t::min()) % T::O2)};
			const auto b_col {static_cast<o2x_t>((rng_() - rng_t::min()) % T::O2)};
			const auto a_box {static_cast<o1x_t>(a_col/T::O1)};
			const auto b_box {static_cast<o1x_t>(b_col/T::O1)};
			if (a_box == b_box) [[unlikely]] { continue; }
			const auto row {static_cast<o2x_t>(h_chute + ((rng_() - rng_t::min()) % T::O1))};
			auto& a_sym {grid.at(row,a_col)};
			auto& b_sym {grid.at(row,b_col)};
			assert(a_sym != b_sym);
			const auto num_missing_syms_resolved {static_cast<signed char>(
				(boxes_has.box_count_sym(a_box,a_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.box_count_sym(a_box,b_sym) == 0 ?  1 : 0) + // improvement
				(boxes_has.box_count_sym(b_box,b_sym) == 1 ? -1 : 0) + // regression
				(boxes_has.box_count_sym(b_box,a_sym) == 0 ?  1 : 0)   // improvement
			)};
			if (num_missing_syms_resolved >= 0) [[unlikely]] { // TODO.low for fun: find out on average at what op_count it starts being unlikely
				num_missing_syms = static_cast<o3i_t>(num_missing_syms - static_cast<o3i_t>(num_missing_syms_resolved));
				--boxes_has.box_count_sym(a_box,a_sym);
				++boxes_has.box_count_sym(a_box,b_sym);
				--boxes_has.box_count_sym(b_box,b_sym);
				++boxes_has.box_count_sym(b_box,a_sym);
				std::swap(a_sym, b_sym);
			}
			// ++op_count;
		}
		// std::cout << "\n" << op_count << ", ";
	}


	template<Order O> requires(is_order_compiled(O))
	struct CountSymsInInvalidCol final {
		using T = Ints<O>;
		using V = typename T::o2i_smol_t;
		using o3i_t = typename T::o3i_t;
		[[nodiscard]] o3i_t count_num_missing_syms() const noexcept { return static_cast<o3i_t>(std::ranges::count(store_, V{0})); }
		template<class T_col, class T_sym> requires(Any_o1x<O, T_col> && Any_o2x<O, T_sym>)
		[[nodiscard]] const V& col_count_sym(const T_col col, const T_sym sym) const noexcept { return store_[(T::O1*sym)+col]; }
		template<class T_col, class T_sym> requires(Any_o1x<O, T_col> && Any_o2x<O, T_sym>)
		[[nodiscard]]       V& col_count_sym(const T_col col, const T_sym sym)       noexcept { return store_[(T::O1*sym)+col]; }
	private:
		// rows for each symbol, entry-in-row for each col.
		std::array<V, T::O3> store_ {0};
	};

	template<Order O> requires(is_order_compiled(O))
	void make_cols_valid(Grid<O>& grid, const typename Ints<O>::o2i_t v_chute, rng_t& rng_) noexcept {
		using T = Ints<O>;
		using o1x_t = typename T::o1x_t;
		using o1i_t = typename T::o1i_t;
		using o2x_t = typename T::o2x_t;
		using o2i_t = typename T::o2i_t;
		using o3i_t = typename T::o3i_t;

		// unsigned long long op_count {0};
		CountSymsInInvalidCol<O> cols_has {};
		for (o2i_t row {0}; row < T::O2; ++row) {
		for (o1i_t box_col {0}; box_col < T::O1; ++box_col) {
			++(cols_has.col_count_sym(box_col, grid.at(row, static_cast<o2x_t>(v_chute + box_col))));
		}}
		o3i_t num_missing_syms {cols_has.count_num_missing_syms()};
		while (num_missing_syms != 0) [[likely]] {
			const auto a_col {static_cast<o1x_t>((rng_() - rng_t::min()) % T::O1)};
			const auto b_col {static_cast<o1x_t>((rng_() - rng_t::min()) % T::O1)};
			if (a_col == b_col) [[unlikely]] { continue; }
			const auto row {static_cast<o2x_t>((rng_() - rng_t::min()) % (T::O2))};
			auto& a_sym {grid.at(row, static_cast<o2i_t>(v_chute + a_col))};
			auto& b_sym {grid.at(row, static_cast<o2i_t>(v_chute + b_col))};
			assert(a_sym != b_sym);
			const auto num_missing_syms_resolved {static_cast<signed char>(
				(cols_has.col_count_sym(a_col,a_sym) == 1 ? -1 : 0) + // regression
				(cols_has.col_count_sym(a_col,b_sym) == 0 ?  1 : 0) + // improvement
				(cols_has.col_count_sym(b_col,b_sym) == 1 ? -1 : 0) + // regression
				(cols_has.col_count_sym(b_col,a_sym) == 0 ?  1 : 0)   // improvement
			)};
			if (num_missing_syms_resolved >= 0) [[unlikely]] {
				num_missing_syms = static_cast<o3i_t>(num_missing_syms - static_cast<o3i_t>(num_missing_syms_resolved));
				--cols_has.col_count_sym(a_col,a_sym);
				++cols_has.col_count_sym(a_col,b_sym);
				--cols_has.col_count_sym(b_col,b_sym);
				++cols_has.col_count_sym(b_col,a_sym);
				std::swap(a_sym, b_sym);
			}
			// ++op_count;
		}
		// std::cout << op_count;
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void generate(Grid<O>& grid, SharedRng& shared_rng) noexcept { // NOLINT(readability-function-cognitive-complexity) *laughs in cognitive complexity of 72
		using T = Ints<O>;
		using o2i_t = typename T::o2i_t;
		{
			std::array<grid_val_t<O>, T::O2> example_row;
			std::iota(example_row.begin(), example_row.end(), grid_val_t<O>{0});
			for (o2i_t row {0}; row < T::O2; ++row) {
				const auto span {grid.row_span_at(row)};
				std::copy(example_row.cbegin(), example_row.cend(), span.begin());
			}
		}
		rng_t rng_; // NOLINT(cert-msc32-c,cert-msc51-cpp) deferred seeding
		{
			std::lock_guard lock_guard {shared_rng.mutex};
			rng_.seed(static_cast<unsigned int>(shared_rng.rng()));
			for (o2i_t row {0}; row < T::O2; ++row) {
				const auto row_sp {grid.row_span_at(row)};
				std::ranges::shuffle(row_sp, shared_rng.rng);
			}
			// TODO.try should the shuffle just use `rng_`? Note: A data-parallel implementation would be much better that way.
		}
		/* Note: when making boxes valid, keeping one line untouched works,
		but is actually slower. same for making columns valid and one box. */

		for (o2i_t h_chute {0}; h_chute < T::O2; h_chute += T::O1) {
			make_boxes_valid(grid, h_chute, rng_);
		}
		for (o2i_t v_chute {0}; v_chute < T::O2; v_chute += T::O1) {
			make_cols_valid(grid, v_chute, rng_);
		}

		assert(grid_follows_rule<O>(grid));
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void generate<O_>(Grid<O_>&, SharedRng&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	void generate(Grid& vis_sink, SharedRng& shared_rng) noexcept {
		return std::visit([&](auto& mono_sink) {
			return mono::generate(mono_sink, shared_rng);
		}, vis_sink.get_mono_variant());
	}
}