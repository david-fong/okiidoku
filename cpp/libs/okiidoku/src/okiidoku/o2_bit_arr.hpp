// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_O2_BIT_ARR
#define HPP_OKIIDOKU_O2_BIT_ARR

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <algorithm>   // min
#include <array>
#include <iterator>    // input_iterator_tag, default_sentinel
#include <bit>         // countr_zero
#include <cstdint>     // uint..._t
#include <climits>     // CHAR_BIT
#include <compare>     // strong_ordering
#include <type_traits> // conditional_t, is_aggregate_v

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct O2BitArr {
	private:
		using T = Ints<O>;
		using o2xs_t = T::o2xs_t;
		using o2x_t  = T::o2x_t;
		using o2i_t  = T::o2i_t;
	public:
		// TODO.low investigate ways to store small but expand to a fast(er) int type
		// when doing bit-twiddling operations and whether the tradeoff is good pareto-wise.
		using word_t =
			std::conditional_t<(O*O <= 32), std::uint32_t,
			std::uint64_t
		>;
		// since last measured for clang, the above is slightly faster for O=3, with
		// slightly better codegen and slightly bigger code size.
		// using word_t =
		// 	std::conditional_t<(O <= CHAR_BIT), detail::uint_small_for_width_t<T::O2>,
		// 	std::uint_least64_t
		// >;
	private:
		// note: use of one byte is safe for grid orders < 128. that should be fine.
		using word_bit_i_t  = Int<std::min(std::uintmax_t{T::O2}, std::uintmax_t{CHAR_BIT} * sizeof(word_t))>;
		using word_bit_ix_t = Int<word_bit_i_t::max-1u>;
		using word_i_t  = Int<(std::uintmax_t{T::O2} + word_bit_i_t::max-1u) / word_bit_i_t::max>;
		using word_ix_t = Int<word_i_t::max-1u>;
		static constexpr word_bit_i_t word_t_num_bits {word_bit_i_t::max};
		static constexpr word_i_t     num_words       {word_i_t::max};
		static constexpr word_bit_i_t num_excess_bits {(num_words * word_t_num_bits) - T::O2};
		static_assert(  num_words > 0u);
		static_assert(( num_words     * word_t_num_bits) >= T::O2, "enough words"   );
		static_assert(((num_words-1u) * word_t_num_bits)  < T::O2, "no excess words");
		static_assert(word_t_num_bits <= T::O2);

		/** \pre `bit_i < T::O2`. */
		[[nodiscard, gnu::const]]
		static constexpr word_ix_t bit_i_to_word_i(const typename Ints<O>::o2x_t bit_i) noexcept {
			OKIIDOKU_CONTRACT_USE(bit_i < T::O2);
			if constexpr (num_words == 1u) { return word_ix_t{0u}; }
			else {
				const word_ix_t word_i {bit_i / word_t_num_bits};
				OKIIDOKU_CONTRACT_USE(word_i < num_words);
				return word_i;
			}
		}
		/** \pre `bit_i < T::O2`. */
		[[nodiscard, gnu::const]]
		static constexpr word_t word_bit_mask_for_bit_i(const typename Ints<O>::o2x_t bit_i) noexcept {
			OKIIDOKU_CONTRACT_USE(bit_i < T::O2);
			if constexpr (num_words == 1u) {
				return static_cast<word_t>(word_t{1u} << bit_i);
			} else {
				return static_cast<word_t>(word_t{1u} << (bit_i % word_t_num_bits));
			}
		}

	private:
		/** \internal If user follows contracts, excess top bits are always zero. */
		using words_t = std::array<word_t, num_words>;
		words_t words_ {word_t{0u}}; ///< \copydoc words_t

	public:
		constexpr O2BitArr() noexcept = default;

		// Helper for `O2BitArr_ones`.
		static consteval O2BitArr create_ones_() noexcept {
			O2BitArr<O> _ {};
			for (auto& word : _.words_) { word = static_cast<word_t>(~word); }
			_.words_.back() = static_cast<word_t>(_.words_.back() >> O2BitArr<O>::num_excess_bits);
			return _;
		}

		[[nodiscard, gnu::pure]] constexpr friend bool operator==(const O2BitArr& a, const O2BitArr& b) noexcept = default;

		/** count the number of set bits. */
		[[nodiscard, gnu::pure]] o2i_t count() const noexcept;

		/** count the number of set bits below the specified bit index.
		\pre `end < O2`. */
		[[nodiscard, gnu::pure]] o2x_t count_below(o2x_t end) const noexcept;

		/** \pre `at < O2`. */
		[[nodiscard, gnu::pure]] constexpr bool operator[](const typename Ints<O>::o2x_t at) const noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			return (words_[bit_i_to_word_i(at)] & word_bit_mask) != word_t{0u};
		}
		/** \pre `at < O2`. */
		constexpr void set(const typename Ints<O>::o2x_t at) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			words_[bit_i_to_word_i(at)] |= word_bit_mask_for_bit_i(at);
		}
		/** \pre `at < O2`. */
		constexpr void unset(const typename Ints<O>::o2x_t at) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			words_[bit_i_to_word_i(at)] &= static_cast<word_t>(~word_bit_mask_for_bit_i(at));
		}
		/** \pre `at < O2`. */
		constexpr void flip(const typename Ints<O>::o2x_t at) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			words_[bit_i_to_word_i(at)] ^= word_bit_mask_for_bit_i(at);
		}

		void remove(const O2BitArr& to_remove) noexcept {
			for (const auto i : num_words) {
				words_[i] &= static_cast<word_t>(~to_remove.words_[i]);
			}
		}
		void retain_only(const O2BitArr& to_retain) noexcept {
			for (const auto i : num_words) {
				words_[i] &= to_retain.words_[i];
			}
		}

		void unset_all() noexcept {
			words_.fill(0u);
		}

		/** \pre `at < O2`. */
		[[nodiscard, gnu::pure]] static bool test_any3(const o2x_t at, const O2BitArr& a, const O2BitArr& b, const O2BitArr& c) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			// TODO consider rewriting to just logical-or testing each one separately
			return ((a.words_[word_i] | b.words_[word_i] | c.words_[word_i]) & word_bit_mask) != word_t{0u};
		}
		/** \pre `at < O2`. */
		static void set3(const o2x_t at, O2BitArr& a, O2BitArr& b, O2BitArr& c) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			a.words_[word_i] |= word_bit_mask;
			b.words_[word_i] |= word_bit_mask;
			c.words_[word_i] |= word_bit_mask;
		}
		/** \pre `at < O2`. */
		static void unset3(const o2x_t at, O2BitArr& a, O2BitArr& b, O2BitArr& c) noexcept {
			OKIIDOKU_CONTRACT_USE(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_bit_mask_for_bit_i(at)};
			a.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
			b.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
			c.words_[word_i] &= static_cast<word_t>(~word_bit_mask);
		}

		O2BitArr& operator|=(const O2BitArr& rhs) noexcept {
			for (const auto i : num_words) { words_[i] |= rhs.words_[i]; };
			return *this;
		}
		O2BitArr& operator&=(const O2BitArr& rhs) noexcept {
			for (const auto i : num_words) { words_[i] &= rhs.words_[i]; };
			return *this;
		}
		[[nodiscard, gnu::pure]] friend O2BitArr operator|(O2BitArr lhs, const O2BitArr& rhs) noexcept {
			lhs |= rhs; return lhs;
		}
		[[nodiscard, gnu::pure]] friend O2BitArr operator&(O2BitArr lhs, const O2BitArr& rhs) noexcept {
			lhs &= rhs; return lhs;
		}

		[[nodiscard, gnu::pure]] O2BitArr operator~() const noexcept {
			OKIIDOKU_DEFER_INIT O2BitArr inv; // NOLINT(*-init)
			for (const auto i : num_words) { inv.words_[i] = ~words_[i]; };
			inv.words_.back() &= ~word_t{0u} >> O2BitArr<O>::num_excess_bits;
			return *this;
		}

		/** \pre this mask has at least one set bit.
		\note an ugly name for a "sharp knife". */
		// TODO I don't like the ugly name or "sharp knife"-ness. make this return O2 if not-exists, relax(remove) that precondition, and leave it to the caller to use operator* if they know it exists.
		[[nodiscard, gnu::pure]] o2xs_t first_set_bit_require_exists() const noexcept;

		/**
		\pre `set_bit_i < O2` and there are at least `set_bit_i+1` set bits.
		\post `count_below(get_index_of_nth_set_bit(set_bit_i)) == set_bit_i`. */
		[[nodiscard, gnu::pure]] o2x_t get_index_of_nth_set_bit(o2x_t set_bit_i) const noexcept;


		/** use to iterate through set bits of a snapshot of an `O2BitArr`. */
		class Iter {
		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = ::okiidoku::detail::int_fast_for_max_t<T::O2>;
			using value_type = o2x_t;
			using reference  = o2x_t;
			using pointer    = o2x_t;
		private:
			O2BitArr arr_;
			o2i_t i_;
		public:
			explicit Iter(const O2BitArr& arr) noexcept: arr_{arr} { advance(); }
			[[nodiscard, gnu::pure]] constexpr bool not_end() const noexcept { return i_ < T::O2; }
			/** \pre `not_end()` */
			[[nodiscard, gnu::pure]] constexpr o2x_t value() const noexcept {
				return *i_;
			}
			/** \pre `not_end()` */
			void advance() noexcept {
				OKIIDOKU_CONTRACT_USE(i_ < T::O2);
				word_i_t word_i = i_ / word_t_num_bits;
				OKIIDOKU_CONTRACT_USE(word_i < num_words); // should be obvious, but MSVC is struggling :/
				while (word_i < num_words && arr_.words_[word_i] == 0u) /*[[unlikely]]*/ { ++word_i; }
				if (word_i < num_words) [[likely]] {
					OKIIDOKU_CONTRACT_USE(arr_.words_[word_i] != 0u);
					auto& word {arr_.words_[word_i]};
					OKIIDOKU_CONTRACT_USE(((word_i * word_t_num_bits) + std::countr_zero(word)) < T::O2);
					i_ = (word_i * word_t_num_bits) + std::countr_zero(word);
					word &= static_cast<word_t>(word-word_t{1u}); // unset lowest bit
				} else {
					i_ = T::O2;
				}
			}
			reference operator* () const noexcept { return value(); }
			pointer   operator->() const noexcept { return value(); }
			Iter& operator++()    noexcept { advance(); return *this; }
			Iter  operator++(int) noexcept { Iter tmp = *this; ++(*this); return tmp; }
			// [[nodiscard, gnu::pure]] friend bool operator==(const Iter& a, const Iter& b) noexcept { return (a.arr_ == b.arr_); }
			// [[nodiscard, gnu::pure]] friend bool operator!=(const Iter& a, const Iter& b) noexcept { return (a.arr_ != b.arr_); }
			// [[nodiscard, gnu::pure]] friend bool operator==(const Iter& i, [[maybe_unused]] const std::default_sentinel_t s) noexcept { return !i.not_end(); }
			[[nodiscard, gnu::pure]] friend bool operator!=(const Iter& i, [[maybe_unused]] const std::default_sentinel_t s) noexcept { return  i.not_end(); }
			[[nodiscard, gnu::pure]] auto& begin() { return *this; }
			[[nodiscard, gnu::pure]] auto  end()   { return std::default_sentinel; }
		};
		[[nodiscard, gnu::pure]] Iter set_bits() const noexcept {
			return Iter(*this);
		}

		// Defines a strong ordering between masks. Needs design and testing work.
		[[nodiscard, gnu::pure]] static std::strong_ordering cmp_differences(const O2BitArr& a, const O2BitArr& b) noexcept;

		// least-significant bit is the least-significant (left-most) character.
		[[nodiscard, gnu::pure]] OKIIDOKU_KEEP_FOR_DEBUG std::array<char, T::O2> to_chars() const noexcept;
	};
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(!std::is_aggregate_v<O2BitArr<(O_)>>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT


	template<Order O> requires(is_order_compiled(O))
	inline constexpr O2BitArr<O> O2BitArr_ones {O2BitArr<O>::create_ones_()};


	template<Order O>
	struct ChuteBoxMasks {
		using T = Ints<O>;
		/// `[111'000'000, 000'111'000, 000'000'111]`
		static constexpr std::array<O2BitArr<O>, O> row {[]{
			std::array<O2BitArr<O>, O> mask;
			for (const auto chute : T::O1) {
				for (const auto i : T::O1) {
					mask[chute].set((O*chute) + i);
			}	}
			return mask;
		}()};
		/// `[100'100'100, 010'010'010, 001'001'001]`
		static constexpr std::array<O2BitArr<O>, O> col {[]{
			std::array<O2BitArr<O>, O> mask;
			for (const auto chute : T::O1) {
				for (const auto i : T::O1) {
					mask[chute].set((O*i) + chute);
			}	}
			return mask;
		}()};
	};
}
#endif