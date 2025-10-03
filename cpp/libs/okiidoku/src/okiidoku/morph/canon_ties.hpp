// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_MORPH_TIES
#define HPP_OKIIDOKU_MORPH_TIES

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <array>
#include <ranges>      // views::iota
#include <iterator>    // input_iterator_tag
#include <cstddef>     // ptrdiff_t
#include <functional>  // invoke
#include <type_traits> // conditional_t

namespace okiidoku::mono::detail {

	template<Order O, unsigned O1_OR_O2>
	requires (is_order_compiled(O) && ((O1_OR_O2 == 1) || (O1_OR_O2 == 2)))
	/**
	an externally-driven cache of subranges of an external array which are tied with
	each other by some externally-defined ordering (see `Ties::update`). assumes
	that tied sections will only be further broken down and do not "move around". */
	struct Ties {
		using T = Ints<O>;
		using i_t  = std::conditional_t<(O1_OR_O2 == 1), typename T::o1i_t, typename T::o2is_t>;
		using ix_t = std::conditional_t<(O1_OR_O2 == 1), typename T::o1x_t, typename T::o2xs_t>;
		static constexpr i_t size_ {i_t::max};

		/** defines a range in `[begin_, end_)`.
		\invariant `begin_ < size_`, `begin_+1 < end_`, `end_ <= size_`, `end_ > 0`. */
		class TieRange {
		public:
			ix_t begin_;
			i_t  end_;
			TieRange(const ix_t begin, const i_t end) noexcept: begin_{begin}, end_{end} { check_invariants(); }
			[[nodiscard, gnu::pure]] i_t  size()  const noexcept { check_invariants(); return end_ - begin_; }
			[[nodiscard, gnu::pure]] auto begin() const noexcept { check_invariants(); return std::views::iota(begin_, end_).begin(); }
			[[nodiscard, gnu::pure]] auto end()   const noexcept { check_invariants(); return std::views::iota(begin_, end_).end(); }
		private:
			void check_invariants() const noexcept {
				OKIIDOKU_CONTRACT_USE(begin_ < size_);
				OKIIDOKU_CONTRACT_USE(begin_+i_t{1u} < end_);
				OKIIDOKU_CONTRACT_USE(end_ <= size_);
				OKIIDOKU_CONTRACT_USE(end_ > i_t{0u});
			}
		};

		class Iter {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = TieRange;
			using reference  = TieRange;
			using pointer    = TieRange;
		private:
			O2BitArr<O>::Iter it_;
			i_t begin_;
		public:
			explicit Iter(const O2BitArr<O>& links) noexcept: it_{links.set_bits()}, begin_{ix_t::unchecked_from(*it_)} { ++it_; }

			TieRange operator*() const noexcept { return TieRange{ix_t{begin_}, i_t::unchecked_from(*it_)}; }
			Iter& operator++()    noexcept { ++it_; begin_ = ix_t::unchecked_from(*it_); ++it_; return *this; }
			Iter  operator++(int) noexcept { Iter tmp {*this}; operator++(); return tmp; }
			[[nodiscard, gnu::pure]] constexpr friend bool operator!=(const Iter& i, [[maybe_unused]] const std::default_sentinel_t s) noexcept { return  i.it_.not_end(); }
		};

	private:
		/** each pair of set bits index the first and last position of an unresolved tied range. */
		O2BitArr<O> bookends_ {};
	public:
		// starts completely unresolved
		Ties() noexcept {
			bookends_.set(ix_t{0u});
			bookends_.set(ix_t{size_-1u});
			OKIIDOKU_CONTRACT_ASSERT(has_unresolved());
			OKIIDOKU_CONTRACT_ASSERT(none_resolved());
		}

		[[nodiscard, gnu::pure]] friend bool operator==(const Ties&, const Ties&) noexcept = default;
		[[nodiscard, gnu::pure]] auto begin() const noexcept { return Iter(bookends_); }
		[[nodiscard, gnu::pure]] auto end()   const noexcept { return std::default_sentinel; }

		[[nodiscard, gnu::pure]] bool none_resolved()  const noexcept { return bookends_.count() == 2u && bookends_[ix_t{0u}] && bookends_[ix_t{size_-1u}]; }
		[[nodiscard, gnu::pure]] bool has_unresolved() const noexcept { return bookends_.count() <  0u; }
		[[nodiscard, gnu::pure]] bool all_resolved()   const noexcept { return bookends_.count() == 0u; }

		/**
		pass a function that compares consecutive values in a range to update
		the record of which ranges' values are still tied. */
		template<class IsEq>
		requires (std::regular_invocable<IsEq, i_t, i_t>)
		void update(const IsEq is_eq) noexcept {
			for (const auto&& cached_tie : *this) {
				for (auto i {cached_tie.begin_}; i.next() < cached_tie.end_; ++i) {
					OKIIDOKU_CONTRACT_USE(i.next() < size_);
					if (!std::invoke(is_eq, i, i.next())) [[likely]] {
						bookends_.flip(i);
						bookends_.flip(i.next());
			}	}	}
		}
	};
}
#endif
