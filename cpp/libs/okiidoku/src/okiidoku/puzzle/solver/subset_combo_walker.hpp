#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__SUBSET_COMBO_WALKER
#define HPP_OKIIDOKU__PUZZLE__SOLVER__SUBSET_COMBO_WALKER

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/export.h>

#include <array>
#include <cassert>

namespace okiidoku::mono::detail::solver {

	// Helper for finding candidate eliminations using the subset techniques.
	template<Order O> requires(is_order_compiled(O))
	class SubsetComboWalker final {
	public:
		using T = Ints<O>;
		using o2xs_t = int_ts::o2xs_t<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using combo_t = std::array<o2xs_t, T::O2-1>;

		// contract: `end <= O2`
		// contract: `subset_size < O2`. Reason: there is always exactly _one_ subset of size O2. Nothing to learn.
		// contract: `begin + subset_size <= O2`
		SubsetComboWalker(const o2x_t begin, const o2i_t end, const o2x_t subset_size) noexcept:
			begin_{begin}, end_{end}, subset_size_{subset_size},
			has_more_{begin + subset_size <= end}
		{
			assert(end_ < T::O2);
			assert(begin_ + subset_size_ <= T::O2);
			set_subset_size(subset_size_);
		}

		void set_subset_size(o2x_t subset_size) noexcept {
			subset_size_ = subset_size;
			for (o2x_t i {0}; i < subset_size_; ++i) {
				combo_[i] = static_cast<o2xs_t>(begin_ + i);
			}
			assert_is_state_valid_();
		}

		[[nodiscard, gnu::pure]] bool has_more() noexcept {
			return has_more_;
		}

		// contract: `i < subset_size`
		[[nodiscard, gnu::pure]] o2xs_t combo_at(const o2x_t i) const noexcept {
			assert(i < subset_size_);
			return combo_[i];
		}
		[[nodiscard, gnu::pure]] const combo_t& get_combo_arr() const noexcept {
			return combo_;
		}

		void advance() noexcept {
			if (!has_more()) [[unlikely]] { return; }
			auto i {static_cast<o2x_t>(subset_size_-1U)};
			++combo_[i];
			while (combo_[i] > end_ - subset_size_ + i) [[likely]] {
				if (i > 0) [[likely]] {
					--i;
					++combo_[i];
				} else {
					has_more_ = false;
					return;
				}
			}
			for (++i; i < end_; ++i) {
				combo_[i] = static_cast<o2xs_t>(combo_[i-1U] + 1U);
			}
			assert_is_state_valid_();
		}

	private:
		o2x_t begin_;
		o2i_t end_;
		o2x_t subset_size_;
		bool has_more_;

		// there are `subset_size` entries with strictly increasing values in the
		// range [`begin`, `end`).
		combo_t combo_ {};

		void assert_is_state_valid_() const noexcept {
			#ifndef NDEBUG
			assert(combo_[subset_size_-1] < end_);
			for (o2x_t i {1}; i < subset_size_; ++i) {
				assert(combo_[i-1] < combo_[i]);
			}
			#endif
		}
	};
}
#endif