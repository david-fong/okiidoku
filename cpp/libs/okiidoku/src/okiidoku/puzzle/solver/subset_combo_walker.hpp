// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_SOLVER_SUBSET_COMBO_WALKER
#define HPP_OKIIDOKU_PUZZLE_SOLVER_SUBSET_COMBO_WALKER

#include <okiidoku/ints.hpp>
#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>
#include <array>

namespace okiidoku::mono::detail::solver {

	// Helper for finding candidate eliminations using the subset techniques.
	template<Order O> requires(is_order_compiled(O))
	class SubsetComboWalker {
	private:
		using T = Ints<O>;
		using o2xs_t = T::o2xs_t;
		using o2x_t  = T::o2x_t;
		using o2i_t  = T::o2i_t;
		using combo_t = std::array<o2xs_t, T::O2-1u>;
	public:

		// \pre `end <= O2`
		// \pre `naked_subset_size > 0`
		// \pre `naked_subset_size < O2`. Reason: for a puzzle with at least one
		//  solution, there is always exactly one subset of size `O2`.
		// \pre `begin + naked_subset_size <= O2`
		SubsetComboWalker(const o2x_t begin, const o2i_t end, const o2x_t naked_subset_size) noexcept:
			begin_{begin}, end_{end}, naked_subset_size_{naked_subset_size},
			has_more_{(begin + naked_subset_size) <= end}
		{
			OKIIDOKU_CONTRACT_USE(end_ <= T::O2);
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ > 0u);
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ < T::O2);
			OKIIDOKU_CONTRACT_USE(begin_ + naked_subset_size_ <= T::O2);
			if (has_more()) [[likely]] {
				for (const auto i : naked_subset_size_) {
					combo_[i] = o2x_t::unchecked_from(begin_ + i);
			}	}
			assert_is_state_valid_();
		}

		[[nodiscard, gnu::pure]] o2x_t get_naked_subset_size() const noexcept {
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ > 0u);
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ < T::O2);
			return naked_subset_size_;
		}

		[[nodiscard, gnu::pure]] constexpr bool has_more() const noexcept {
			return has_more_;
		}

		/** \pre `i < naked_subset_size` */
		[[nodiscard, gnu::pure]] o2xs_t combo_at(const o2x_t i) const noexcept {
			OKIIDOKU_CONTRACT_USE(i < naked_subset_size_);
			return combo_[i];
		}
		/** \pre `has_more` returns `true`. */
		[[nodiscard, gnu::pure]] auto at_it() const noexcept {
			OKIIDOKU_CONTRACT_ASSERT(has_more());
			return combo_.cbegin();
		}

		/** \pre `has_more` returns `true`. */
		void advance() noexcept {
			OKIIDOKU_CONTRACT_USE(has_more());
			OKIIDOKU_CONTRACT_USE(end_ <= T::O2);
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ > 0u);
			OKIIDOKU_CONTRACT_USE(naked_subset_size_ < T::O2);
			o2x_t i {naked_subset_size_.prev()};
			++combo_[i];
			while (combo_[i] > end_ - naked_subset_size_ + i) [[likely]] {
				if (i > 0u) [[likely]] {
					--i;
					++combo_[i];
				} else {
					has_more_ = false;
					return;
				}
			}
			for (++i; i < naked_subset_size_; ++i) {
				combo_[i] = combo_[i.prev()].next();
			}
			assert_is_state_valid_();
		}

	private:
		o2x_t begin_;
		o2i_t end_;
		o2x_t naked_subset_size_;
		bool has_more_;

		// there are `naked_subset_size` entries with strictly increasing values in the
		// range [`begin`, `end`).
		combo_t combo_ {}; // TODO consider intentionally not default initializing for performance. may need to add NOLINT. We have assertions. hm. did some naive benchmarks and can't really see a consistent difference.

		void assert_is_state_valid_() const noexcept {
			#ifndef NDEBUG
			if ((begin_ + naked_subset_size_) > end_) {
				OKIIDOKU_CONTRACT_ASSERT(!has_more_);
				return;
			}
			OKIIDOKU_CONTRACT_ASSERT(combo_.front() >= begin_);
			OKIIDOKU_CONTRACT_ASSERT(combo_[naked_subset_size_-1u] < end_);
			for (const auto i : naked_subset_size_) {
				OKIIDOKU_CONTRACT_ASSERT(combo_[i-1u] < combo_[i]);
			}
			#endif
		}
	};
}
#endif