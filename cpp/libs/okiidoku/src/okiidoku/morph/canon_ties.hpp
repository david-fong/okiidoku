// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_MORPH_TIES
#define HPP_OKIIDOKU_MORPH_TIES

#include <okiidoku/bit_array.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/detail/util.hpp>

#include <array>
#include <iterator>    // input_iterator_tag
#include <functional>  // invoke
#include <cstddef>     // ptrdiff_t
#include <concepts>    // equivalence_relation
#include <type_traits> // conditional_t

namespace okiidoku::mono::detail {

	template<std::size_t size_> requires(size_ > 0uz)
	/**
	an externally-driven cache of subranges of an external array which are tied with
	each other by some externally-defined ordering (see `Ties::update`). assumes
	that tied sections will only be further broken down and do not "move around". */
	struct Ties final {
		static constexpr std::size_t size {size_};
		using ii_t = Int<size_    , IntKind::small>;
		using ix_t = Int<size_-1uz, IntKind::small>;
	private:
		using bookends_t = BitArray<ii_t::max,IntKind::fast>;
		/** each pair of set bits index the first and last included positions in an unresolved tied range. */
		bookends_t bookends_ {};

	public:
		/** defines a range in `[begin_, end_)`.
		\invariant `begin_ < size`, `begin_+1 < end_`, `end_ <= size`, `end_ > 0`. */
		class TieRange final {
		public:
			ix_t begin_;
			ii_t end_; // one-past the last included position in the tied range.
			constexpr TieRange(const ix_t begin, const ii_t end) noexcept: begin_{begin}, end_{end} { check(); }
			[[nodiscard, gnu::pure]] constexpr ii_t size()  const noexcept { check(); ii_t sz {end_ - begin_}; OKIIDOKU_CONTRACT(sz >= 2u); return sz; }
			[[nodiscard, gnu::pure]] constexpr ii_t begin() const noexcept { check(); return ii_t{begin_}; }
			[[nodiscard, gnu::pure]] constexpr ii_t end()   const noexcept { check(); return end_; }
			[[gnu::always_inline]] constexpr void check() const noexcept {
				begin_.check(); end_.check();
				OKIIDOKU_CONTRACT(begin_ < Ties::size);
				OKIIDOKU_CONTRACT(begin_ < end_.prev());
				OKIIDOKU_CONTRACT(begin_.next() < end_);
				OKIIDOKU_CONTRACT(end_ <= Ties::size);
				OKIIDOKU_CONTRACT(end_ > 0u);
			}
		};

		class Iter final {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = TieRange;
			using reference  = TieRange;
			using pointer    = TieRange;
		private:
			bookends_t::Iter it_;
			ii_t begin_;
		public:
			explicit constexpr Iter(const bookends_t& bookends) noexcept: it_{bookends.set_bits()}, begin_{*it_} { ++it_; }

			[[nodiscard, gnu::pure]] constexpr friend bool operator!=(const Iter& i, [[maybe_unused]] const std::default_sentinel_t s) noexcept { return i.it_.not_end(); }
			[[nodiscard, gnu::pure]] constexpr TieRange operator*() const noexcept { OKIIDOKU_CONTRACT(it_.not_end()); return TieRange{*begin_, (*it_)+int1}; }
			constexpr Iter& operator++() noexcept { ++it_; begin_ = *it_; ++it_; return *this; }
			// Iter  operator++(int) noexcept { Iter tmp {*this}; operator++(); return tmp; } // TODO is this needed?
		};

	public:
		// starts completely unresolved
		Ties() noexcept {
			bookends_.set(ix_t{0u});
			bookends_.set(ix_t{size-1u});
			OKIIDOKU_ASSERT(has_unresolved());
			OKIIDOKU_ASSERT(none_resolved());
		}

		[[nodiscard, gnu::pure]] friend bool operator==(const Ties&, const Ties&) noexcept = default;
		[[nodiscard, gnu::pure]]  constexpr auto begin() const noexcept { return Iter{bookends_}; }
		[[nodiscard, gnu::const]] constexpr auto end()   const noexcept { return std::default_sentinel; }

		[[nodiscard, gnu::pure]] constexpr bool none_resolved()  const noexcept { return bookends_.count() == 2u && bookends_[0u] && bookends_[ix_t{size-1u}]; }
		[[nodiscard, gnu::pure]] constexpr bool has_unresolved() const noexcept { return bookends_.count() >  0u; }
		[[nodiscard, gnu::pure]] constexpr bool all_resolved()   const noexcept { return bookends_.count() == 0u; }

		/**
		pass a function that compares consecutive values in a range to update
		the record of which ranges' values are still tied. */
		void update(const std::equivalence_relation<ix_t,ix_t> auto& is_eq) noexcept {
			for (const auto&& cached_tie : *this) {
				for (ii_t i {cached_tie.begin_}; i.next() != cached_tie.end_; ++i) {
					OKIIDOKU_CONTRACT(i.next() < size);
					if (!std::invoke(is_eq, *i, *i.next())) [[likely]] {
						bookends_.flip(*i);
						bookends_.flip(*i.next());
			}	}	}
		}
	};
}
#endif
