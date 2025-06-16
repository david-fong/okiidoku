// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__TIES
#define HPP_OKIIDOKU__MORPH__TIES

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <range/v3/view/iota.hpp>

#include <algorithm>
#include <array>
#include <type_traits>

namespace okiidoku::mono::detail {

	template<Order O, unsigned O1_OR_O2>
	requires (is_order_compiled(O) && ((O1_OR_O2 == 1) || (O1_OR_O2 == 2)))
	struct TieLinks final {
		using T = Ints<O>;
		static constexpr std::size_t size_ {(O1_OR_O2 == 1) ? T::O1 : T::O2};
		using link_t = std::conditional_t<(O1_OR_O2 == 1), int_ts::o1i_t<O>, int_ts::o2is_t<O>>;
		using links_t = std::array<link_t, size_>;

		class Range final {
		public:
			link_t begin_;
			link_t end_;
			Range(const link_t begin, const link_t end) noexcept: begin_{begin}, end_{end} { OKIIDOKU_CONTRACT_USE(begin_ < end_); }
			[[nodiscard]] link_t size() const noexcept { OKIIDOKU_CONTRACT_USE(begin_ < end_); return static_cast<link_t>(end_ - begin_); }
			auto begin() const noexcept { OKIIDOKU_CONTRACT_USE(begin_ < end_); return ranges::views::iota(begin_, end_).begin(); }
			auto end()   const noexcept { OKIIDOKU_CONTRACT_USE(begin_ < end_); return ranges::views::iota(begin_, end_).end(); }
		};

		class Iterator final {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Range;
			using reference = value_type&;
			using pointer = value_type*;
		private:
			const links_t& links_;
			link_t i_;
		public:
			Iterator(const links_t& links, link_t i = 0) noexcept: links_(links), i_{i} {}

			Range operator*()  const noexcept { return Range{i_, links_[i_]}; }
			Range operator->() const noexcept { return Range{i_, links_[i_]}; }
			Iterator& operator++() noexcept { OKIIDOKU_CONTRACT_ASSERT(links_[i_] > i_); i_ = links_[i_]; return *this; }
			Iterator operator++(int) noexcept { Iterator tmp = *this; ++(*this); return tmp; }
			[[nodiscard, gnu::pure]] friend bool operator==(const Iterator& a, const Iterator& b) noexcept { return (&a.links_ == &b.links_) && (a.i_ == b.i_); }
			[[nodiscard, gnu::pure]] friend bool operator!=(const Iterator& a, const Iterator& b) noexcept { return (&a.links_ != &b.links_) || (a.i_ != b.i_); }
		};
	private:
		links_t links_ {0};
	public:
		// starts completely unresolved
		TieLinks() noexcept: links_{[]{ links_t _{0}; _[0] = size_; return _; }()} {}
		[[nodiscard, gnu::pure]] friend bool operator==(const TieLinks&, const TieLinks&) noexcept = default;
		Iterator begin() const noexcept { return Iterator(links_); }
		Iterator end()   const noexcept { return Iterator(links_, size_); }

		[[nodiscard, gnu::pure]] bool has_unresolved() const noexcept { return std::any_of(links_.cbegin(), links_.cend(), [](const auto& e){ return e == 0; }); }
		[[nodiscard, gnu::pure]] bool is_completely_unresolved() const noexcept { return links_[0] == size_; }

		// pass a function that compares consecutive values in a range to update
		// the record of which ranges' values are still tied.
		template<class IsEq>
		// requires (std::regular_invocable<IsEq, link_t, link_t>)
		void update(const IsEq is_eq) noexcept {
			for (const auto tie : *this) {
				OKIIDOKU_CONTRACT_USE(tie.begin_ < tie.end_);
				auto cursor {tie.begin_};
				for (auto i {static_cast<link_t>(cursor+1)}; i < tie.end_; ++i) {
					OKIIDOKU_CONTRACT_USE(cursor < i);
					if (!std::invoke(is_eq, static_cast<link_t>(i-1), i)) [[likely]] {
						links_[cursor] = i;
						cursor = i;
				}	}
				links_[cursor] = tie.end_;
			}
		}
	};
}
#endif
