#include <okiidoku/grid.hpp>

namespace okiidoku {

	template<Order O>
	requires(is_order_compiled(O))
	bool grid_follows_rule(const grid_const_span_t<O> grid) noexcept {
		using o2i_t = typename traits<O>::o2i_t;
		using has_mask_t = typename traits<O>::o2_bits_fast;
		static constexpr o2i_t O2 {O*O};

		std::array<has_mask_t, O2> rows_has_ {};
		std::array<has_mask_t, O2> cols_has_ {};
		std::array<has_mask_t, O2> blks_has_ {};

		for (o2i_t row {0}; row < O2; ++row) {
		for (o2i_t col {0}; col < O2; ++col) {
			const auto val {grid[(O2*row) + col]};
			assert(val <= O2);
			if (val == O2) { continue; }

			auto& row_has = rows_has_[row];
			auto& col_has = cols_has_[col];
			auto& blk_has = blks_has_[rmi_to_blk<O>(row, col)];

			const has_mask_t try_val_mask {has_mask_t{1} << val};
			const has_mask_t t_has {row_has | col_has | blk_has};
			if ((t_has & try_val_mask).any()) [[unlikely]] {
				return false;
			} else {
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
			}
		}}
		return true;
	}


	template<Order O>
	requires(is_order_compiled(O))
	bool grid_is_filled(const grid_const_span_t<O> grid) noexcept {
		static constexpr typename traits<O>::o2i_t O2 {O*O};
		for (auto val : grid) {
			assert(val <= O2);
			if (val >= O2) { return false; }
		}
		return true;
	}


	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		template bool grid_follows_rule<O_>(grid_const_span_t<O_>) noexcept; \
		template bool grid_is_filled<O_>(grid_const_span_t<O_>) noexcept;
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL
}