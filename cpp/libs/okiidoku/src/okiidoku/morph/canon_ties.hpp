#ifndef HPP_OKIIDOKU__MORPH__TIES
#define HPP_OKIIDOKU__MORPH__TIES

#include <okiidoku/traits.hpp>
#include <okiidoku/prelude.hpp>

#include <algorithm>
#include <ranges>
#include <array>
#include <type_traits>

namespace okiidoku::mono::morph {

	template<Order O, unsigned O1_OR_O2>
	requires (O1_OR_O2 == 1) || (O1_OR_O2 == 2)
	struct TieLinks final {
		static constexpr size_t size_ = (O1_OR_O2 == 1) ? traits<O>::O1 : traits<O>::O2;
		using link_t = std::conditional_t<(O1_OR_O2 == 1), typename traits<O>::o1i_t, typename traits<O>::o2i_smol_t>;
		using links_t = std::array<link_t, size_>;

		struct Range final {
			link_t begin_;
			link_t end_;
			[[nodiscard]] constexpr link_t size() const noexcept { return end_ - begin_; }
			constexpr auto begin() const { return std::views::iota(begin_, end_).begin(); }
			constexpr auto end()   const { return std::views::iota(begin_, end_).end(); }
		};
		struct Iterator final {
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Range;
			using reference = value_type&;
			using pointer = value_type*;
		private:
			const links_t& links_;
			link_t i_;
		public:
			constexpr Iterator(const links_t& links, link_t i = 0): links_(links), i_{i} {}

			Range operator*() const { return {i_, links_[i_]}; }
			Range operator->() const { return {i_, links_[i_]}; }
			constexpr Iterator& operator++() { i_ = links_[i_]; return *this; }  
			constexpr Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
			constexpr friend bool operator==(const Iterator& a, const Iterator& b) { return (&a.links_ == &b.links_) && (a.i_ == b.i_); }
			constexpr friend bool operator!=(const Iterator& a, const Iterator& b) { return (&a.links_ != &b.links_) || (a.i_ != b.i_); }
		};
	private:
		links_t links_ {0};
	public:
		constexpr TieLinks(): links_{[]{ links_t _{0}; _[0] = size_; return _; }()} {}
		constexpr bool operator==(const TieLinks&) const = default;
		constexpr Iterator begin() const { return Iterator(links_); }
		constexpr Iterator end() const { return Iterator(links_, size_); }

		[[nodiscard]] bool has_unresolved() const { return std::ranges::any_of(links_, [](const auto& e){ return e == 0; }); }
		[[nodiscard]] bool is_completely_unresolved() const noexcept { return links_[0] == size_; }

		template<class IsEq>
		// requires (std::regular_invocable<IsEq, link_t, link_t>)
		void update(IsEq is_eq) {
			for (const auto tie : *this) {
				auto begin {tie.begin_};
				for (auto i {static_cast<link_t>(begin+1)}; i < tie.end_; ++i) {
					if (!std::invoke(is_eq, i-1, i)) [[likely]] {
						links_[begin] = i;
						begin = i;
				}	}
				links_[begin] = tie.end_;
			}
		}
	};
}
#endif