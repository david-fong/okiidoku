// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__TIES
#define HPP_OKIIDOKU__MORPH__TIES

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <array>
#include <ranges>      // views::iota
#include <iterator>    // input_iterator_tag
#include <functional>  // invoke
#include <type_traits> // conditional_t

namespace okiidoku::mono::detail {

	template<Order O, unsigned O1_OR_O2>
	requires (is_order_compiled(O) && ((O1_OR_O2 == 1) || (O1_OR_O2 == 2)))
	/**
	an externally-driven cache of subranges of an external array which are tied with
	each other by some externally-defined ordering (see `TieLinks::update`). assumes
	that tied sections will only be further broken down and do not "move around". */
	// TODO try to implement this in terms of O2BitArray and iterator in terms of O2BitArray::SetBitsWalker
	struct TieLinks final {
		using T = Ints<O>;
		static constexpr std::size_t size_ {(O1_OR_O2 == 1) ? T::O1 : T::O2};
		using link_t = std::conditional_t<(O1_OR_O2 == 1), int_ts::o1i_t<O>, int_ts::o2is_t<O>>;
		using links_t = std::array<link_t, size_>;

		/** defines a range in `[begin_, end_)`
		\invariant `begin_ < end_`, `begin_ < size_` */
		class TieRange final {
		public:
			link_t begin_;
			link_t end_;
			TieRange(const link_t begin, const link_t end) noexcept: begin_{begin}, end_{end} {
				check_invariants();
			}
			[[nodiscard, gnu::pure]] link_t size() const noexcept { OKIIDOKU_CONTRACT_USE(begin_ < end_); return static_cast<link_t>(end_ - begin_); }
			auto begin() const noexcept { check_invariants(); return std::views::iota(begin_, end_).begin(); }
			auto end()   const noexcept { check_invariants(); return std::views::iota(begin_, end_).end(); }
		private:
			void check_invariants() const noexcept {
				OKIIDOKU_CONTRACT_USE(begin_ < end_);
				OKIIDOKU_CONTRACT_USE(begin_ < size_);
				OKIIDOKU_CONTRACT_USE(end_ <= size_);
				OKIIDOKU_CONTRACT_USE(end_ > link_t{0});
			}
		};

		class Iterator final {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = TieRange;
			using reference = value_type&;
			using pointer = value_type*;
		private:
			const links_t& links_;
			link_t i_;
		public:
			/** to make an end sentinel, pass `i = TieLinks<>::size_`. */
			Iterator(const links_t& links, link_t i = 0) noexcept: links_(links), i_{i} {}

			TieRange operator*()  const noexcept { return TieRange{i_, links_[i_]}; }
			TieRange operator->() const noexcept { return TieRange{i_, links_[i_]}; }
			Iterator& operator++() noexcept { OKIIDOKU_CONTRACT_ASSERT(links_[i_] > i_); i_ = links_[i_]; return *this; }
			Iterator operator++(int) noexcept { Iterator tmp = *this; ++(*this); return tmp; }
			[[nodiscard, gnu::pure]] friend bool operator==(const Iterator& a, const Iterator& b) noexcept { return (&a.links_ == &b.links_) && (a.i_ == b.i_); }
			[[nodiscard, gnu::pure]] friend bool operator!=(const Iterator& a, const Iterator& b) noexcept { return (&a.links_ != &b.links_) || (a.i_ != b.i_); }
		};

	private:
		/** `[i, links_[i]) describes a tied subrange of the external array. */
		links_t links_;
	public:
		// starts completely unresolved
		TieLinks() noexcept {
			links_[0] = size_;
			links_.back() = static_cast<link_t>(0);
			OKIIDOKU_CONTRACT_ASSERT(has_unresolved());
			OKIIDOKU_CONTRACT_ASSERT(is_completely_unresolved());
		}

		[[nodiscard, gnu::pure]] friend bool operator==(const TieLinks&, const TieLinks&) noexcept = default;
		[[gnu::pure]] Iterator begin() const noexcept { return Iterator(links_); }
		[[gnu::pure]] Iterator end()   const noexcept { return Iterator(links_, size_); }

		[[nodiscard, gnu::pure]] bool has_unresolved() const noexcept { return links_.back() != size_; }
		[[nodiscard, gnu::pure]] bool is_completely_unresolved() const noexcept { return links_[0] == size_; }

		/**
		pass a function that compares consecutive values in a range to update
		the record of which ranges' values are still tied. */
		template<class IsEq>
		// requires (std::regular_invocable<IsEq, link_t, link_t>)
		void update(const IsEq is_eq) noexcept {
			for (const auto tie : *this) {
				OKIIDOKU_CONTRACT_USE(tie.begin_ < tie.end_);
				auto sub_tie_begin {tie.begin_};
				for (
					auto sub_tie_end {static_cast<link_t>(sub_tie_begin+1)};
					sub_tie_end < tie.end_;
					++sub_tie_end
				) {
					OKIIDOKU_CONTRACT_USE(sub_tie_begin < sub_tie_end);
					// check for newly broken tie:
					if (!std::invoke(is_eq, static_cast<link_t>(sub_tie_end-1), sub_tie_end)) [[likely]] {
						links_[sub_tie_begin] = sub_tie_end;
						sub_tie_begin = sub_tie_end;
					} else {
						// basic self-consistency check for the is_eq function
						OKIIDOKU_CONTRACT_ASSERT(std::invoke(is_eq, sub_tie_begin, sub_tie_end));
					}
				}
				links_[sub_tie_begin] = tie.end_;
			}
		}
	};
}
#endif
