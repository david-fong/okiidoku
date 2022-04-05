#ifndef HPP_OKIIDOKU__MORPH__TIES
#define HPP_OKIIDOKU__MORPH__TIES

#include <okiidoku/traits.hpp>
#include <okiidoku/order.hpp>

#include <algorithm>
#include <array>
#include <type_traits>

namespace okiidoku::morph {

	template<Order O, unsigned O1_OR_O2>
	requires (O1_OR_O2 == 1) || (O1_OR_O2 == 2)
	struct TieLinks final {
		using link_t = std::conditional_t<O1_OR_O2 == 1, traits<O>::o1i_t, traits<O>::o2i_smol_t>;
		using links_t = std::array<link_t, size_>;

		struct Range final {
			link_t begin;
			link_t end;
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
			Iterator& operator++() { i_ = links_[i_]; return *this; }  
			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
			friend bool operator==(const Iterator& a, const Iterator& b) { return (&a.links_ == &b.links_) && (a.i_ == b.i_); }
			friend bool operator!=(const Iterator& a, const Iterator& b) { return !operator==(a, b); }
		};
	private:
		static constexpr size_ = O1_OR_O2 == 1 ? traits<O>::O1 : traits<O>::O2;
		links_t links_ {0};
	public:
		constexpr TieLinks(): links_{[]{ links_t _{0}; _[0] = size_; return _; }} {}
		bool has_unresolved() const { return std::ranges::any_of(links_, [](const auto& e){ return e == 0; }); }
		Iterator begin() const { return Iterator(this.links_); }
		Iterator end() const { return Iterator(this.links_, size_); }
	};
}
#endif