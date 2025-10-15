// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_BIT_ARRAY
#define HPP_OKIIDOKU_BIT_ARRAY
#include <okiidoku/detail/util.hpp>

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <array>
#include <iterator>    // input_iterator_tag, default_sentinel
#include <bit>         // countr_zero
#include <cstdint>     // uint..._t
#include <climits>     // CHAR_BIT
// #include <compare>     // strong_ordering
#include <type_traits> // conditional_t

namespace okiidoku {

	template<std::uintmax_t width_, IntKind kind_ = IntKind::small>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	struct BitArray final {
	public:
		static constexpr std::uintmax_t width {width_};
		static constexpr IntKind kind {kind_};
		using bit_ii_t = Int<width_   , IntKind::fast>;
		using bit_ix_t = Int<width_-1u, IntKind::fast>;
		using word_t =
			std::conditional_t<(kind == IntKind::small && width_ <= static_cast<std::uintmax_t>(std::bit_width(UINTMAX_MAX))), ::okiidoku::detail::uint_small_for_width_t<width_>,
			std::conditional_t<(width_ <= 32u), std::uint32_t, std::uint64_t
		>>;
	private:
		static constexpr unsigned char word_width {sizeof(word_t) * std::size_t{CHAR_BIT}};
		using word_ii_t = Int<(width_ + (word_width-1u)) / word_width>; // i.e. `round_up(width / word_width)`
		using word_ix_t = Int<word_ii_t::max-1u>;
		using word_bit_ii_t = Int<(width_ < word_width) ? width_ : word_width>;
		static constexpr     word_ii_t::constant_t num_words     {};
		static constexpr word_bit_ii_t::constant_t num_word_bits {};
		static constexpr Int<(num_words*word_width)-width_,IntKind::constant> num_excess_bits {};
		static_assert(  num_words > 0u);
		static_assert(( num_words     * num_word_bits) >= width_, "enough words"   );
		static_assert(((num_words-1u) * num_word_bits)  < width_, "no excess words");
		static_assert(num_word_bits <= width);

		/** \pre `bit_i < width`. */
		[[nodiscard, gnu::const]]
		static constexpr word_ix_t bit_i_to_word_i(const bit_ix_t bit_i) noexcept {
			bit_i.check();
			if constexpr (num_words == 1u) { return word_ix_t{0u}; }
			else {
				const word_ix_t word_i {bit_i / num_word_bits};
				word_i.check(); OKIIDOKU_CONTRACT(word_i * num_word_bits < width);
				return word_i;
			}
		}
		/** \pre `bit_i < width`. */
		[[nodiscard, gnu::const]]
		static constexpr word_t word_bit_mask_for_bit_i(const bit_ix_t bit_i) noexcept {
			bit_i.check();
			if constexpr (num_words == 1u) {
				return static_cast<word_t>(word_t{1u} << bit_i);
			} else {
				return static_cast<word_t>(word_t{1u} << (bit_i % num_word_bits));
			}
		}

	private:
		/** \internal If user follows contracts, excess top bits are always zero. */
		std::array<word_t, num_words> words_ {}; // zero-init

	public:
		OKIIDOKU_KEEP_FOR_DEBUG constexpr BitArray() noexcept = default;

		[[nodiscard, gnu::pure]]  constexpr friend bool operator==(const BitArray& a, const BitArray& b) noexcept requires(num_words >  1u) = default;
		[[nodiscard, gnu::const]] constexpr friend bool operator==(      BitArray  a,       BitArray  b) noexcept requires(num_words == 1u) = default;

		/** count the number of set bits. */
		[[nodiscard, gnu::pure]] bit_ii_t count() const noexcept;

		/** count the number of set bits below the specified bit index.
		\pre `end < O2`.
		\post `nth_set_bit(count_below(end)) == end`. */
		[[nodiscard, gnu::pure]] bit_ix_t count_below(bit_ix_t end) const noexcept;

		/** \pre `at < O2`. */
		[[nodiscard, gnu::pure]] constexpr
		bool operator[](const bit_ix_t at) const noexcept {
			at.check();
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			return (words_[bit_i_to_word_i(at)] & word_bit_mask) != word_t{0u};
		}
		/** \pre `at < O2`. */
		constexpr void set(const bit_ix_t at) noexcept {
			at.check();
			words_[bit_i_to_word_i(at)] |= word_bit_mask_for_bit_i(at);
		}
		/** \pre `at < O2`. */
		constexpr void unset(const bit_ix_t at) noexcept {
			at.check();
			words_[bit_i_to_word_i(at)] &= static_cast<word_t>(~word_bit_mask_for_bit_i(at));
		}
		/** \pre `at < O2`. */
		constexpr void flip(const bit_ix_t at) noexcept {
			at.check();
			words_[bit_i_to_word_i(at)] ^= word_bit_mask_for_bit_i(at);
		}

		void remove(const BitArray& to_remove) noexcept {
			for (const auto i : num_words) {
				words_[i] &= static_cast<word_t>(~to_remove.words_[i]);
			}
		}
		void retain_only(const BitArray& to_retain) noexcept {
			for (const auto i : num_words) {
				words_[i] &= to_retain.words_[i];
			}
		}

		void unset_all() noexcept {
			words_.fill(0u);
		}

		/** \pre `at < O2`. */
		[[nodiscard, gnu::pure]] static bool test_any3(const bit_ix_t at, const BitArray& a, const BitArray& b, const BitArray& c) noexcept {
			at.check();
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			// TODO consider rewriting to just logical-or testing each one separately
			return ((a.words_[word_i] | b.words_[word_i] | c.words_[word_i]) & word_bit_mask) != word_t{0u};
		}
		/** \pre `at < O2`. */
		static void set3(const bit_ix_t at, BitArray& a, BitArray& b, BitArray& c) noexcept {
			at.check();
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			a.words_[word_i] |= word_bit_mask;
			b.words_[word_i] |= word_bit_mask;
			c.words_[word_i] |= word_bit_mask;
		}
		/** \pre `at < O2`. */
		static void unset3(const bit_ix_t at, BitArray& a, BitArray& b, BitArray& c) noexcept {
			at.check();
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			a.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
			b.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
			c.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
		}

		BitArray& operator|=(const BitArray& rhs) noexcept {
			for (const auto i : num_words) { words_[i] |= rhs.words_[i]; };
			return *this;
		}
		BitArray& operator&=(const BitArray& rhs) noexcept {
			for (const auto i : num_words) { words_[i] &= rhs.words_[i]; };
			return *this;
		}
		[[nodiscard, gnu::pure]] friend BitArray operator|(BitArray lhs, const BitArray& rhs) noexcept {
			lhs |= rhs; return lhs;
		}
		[[nodiscard, gnu::pure]] friend BitArray operator&(BitArray lhs, const BitArray& rhs) noexcept {
			lhs &= rhs; return lhs;
		}

		[[nodiscard, gnu::pure]] constexpr
		BitArray operator~() const noexcept {
			BitArray inv OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
			for (const auto i : num_words) { inv.words_[i] = ~words_[i]; };
			inv.words_.back() &= static_cast<word_t>(static_cast<word_t>(~word_t{0u}) >> num_excess_bits);
			return inv;
		}

		/** \returns `O2` if no bits are set. otherwise, the index of the first set bit. */
		[[nodiscard, gnu::pure]] bit_ii_t first_set_bit() const noexcept;

		/**
		\pre `set_bit_i < O2` and there are at least `set_bit_i+1` set bits.
		\post `count_below(nth_set_bit(set_bit_i)) == set_bit_i`. */
		[[nodiscard, gnu::pure]] bit_ix_t nth_set_bit(bit_ix_t set_bit_i) const noexcept;


		/** use to iterate through set bits of a snapshot of an `BitArray`. */
		class Iter final {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = ::okiidoku::detail::int_fast_for_max_t<static_cast<std::intmax_t>(width_)>;
			using value_type = bit_ix_t;
			using reference  = bit_ix_t;
			using pointer    = bit_ix_t;
		private:
			BitArray arr_;
			bit_ii_t i_;
		public:
			explicit Iter(const BitArray& arr) noexcept: arr_{arr} { advance(); }
			[[nodiscard, gnu::pure]] constexpr bool not_end() const noexcept { return i_ < width; }
			/** \pre `not_end()` */
			[[nodiscard, gnu::pure]] constexpr bit_ix_t value() const noexcept {
				return *i_;
			}
			/**
			\pre `not_end()`
			\post has advanced to the bit index of the next set bit. */
			void advance() noexcept {
				OKIIDOKU_CONTRACT(i_ < width); OKIIDOKU_CONTRACT(not_end());
				const word_ii_t word_i {[&]noexcept{
					if constexpr (num_words == 1u) {
						if (arr_.words_.front() == 0u) [[unlikely]] { return 1u; }
						else { return 0u; }
					} else {
						word_ii_t w_i {bit_i_to_word_i(i_)};
						while (w_i <= num_words && arr_.words_[w_i] == 0u) /*[[unlikely]]*/ { ++w_i; }
						return w_i;
					}
				}()};
				if (word_i < num_words) [[likely]] {
					auto& word {arr_.words_[word_i]};
					OKIIDOKU_CONTRACT(word != 0u);
					i_ = (word_i * num_word_bits) + std::countr_zero(word);
					word &= static_cast<word_t>(word-word_t{1u}); // unset lowest bit
				} else {
					i_ = width;
				}
			}
			[[gnu::pure]] reference operator* () const noexcept { OKIIDOKU_CONTRACT(not_end()); return value(); }
			[[gnu::pure]] pointer   operator->() const noexcept { OKIIDOKU_CONTRACT(not_end()); return value(); }
			Iter& operator++()    noexcept { advance(); return *this; }
			Iter  operator++(int) noexcept { Iter tmp = *this; ++(*this); return tmp; }
			[[nodiscard, gnu::pure]] friend bool operator!=(const Iter& i, [[maybe_unused]] const std::default_sentinel_t s) noexcept { return i.not_end(); }
			[[nodiscard, gnu::pure]] auto& begin() { return *this; }
			[[nodiscard, gnu::const]] auto end()   { return std::default_sentinel; }
		};
		[[nodiscard, gnu::pure]] Iter set_bits() const noexcept {
			return Iter(*this);
		}

		// Defines a strong ordering between masks. Needs design and testing work.
		// TODO this should probably be declared as a friend operator<=>
		// [[nodiscard, gnu::pure]] static std::strong_ordering cmp_differences(const BitArray& a, const BitArray& b) noexcept;

		// least-significant bit is the front character.
		[[nodiscard, gnu::pure]] OKIIDOKU_KEEP_FOR_DEBUG std::array<char, width_> to_chars() const noexcept;
	};
}


namespace okiidoku::mono {

	template<Order O, IntKind kind_ = IntKind::small> requires(is_order_compiled(O))
	using O2BitArr = BitArray<std::uintmax_t{O*O}, kind_>;

	template<Order O> requires(is_order_compiled(O))
	inline constexpr BitArray<std::uintmax_t{O*O}> O2BitArr_ones {~(BitArray<std::uintmax_t{O*O}>{})};


	template<Order O> requires(is_order_compiled(O))
	struct ChuteBoxMasks final {
		using T = Ints<O>;
		/// `[111'000'000, 000'111'000, 000'000'111]`
		static constexpr std::array<BitArray<std::uintmax_t{O*O}>, O> row {[]{
			std::array<BitArray<std::uintmax_t{O*O}>, O> mask;
			for (const auto chute : T::O1) {
				for (const auto i : T::O1) {
					mask[chute].set((O*chute) + i);
			}	}
			return mask;
		}()};
		/// `[100'100'100, 010'010'010, 001'001'001]`
		static constexpr std::array<BitArray<std::uintmax_t{O*O}>, O> col {[]{
			std::array<BitArray<O>, O> mask;
			for (const auto chute : T::O1) {
				for (const auto i : T::O1) {
					mask[chute].set((O*i) + chute);
			}	}
			return mask;
		}()};
	};
}
#endif