#ifndef HPP_OKIIDOKU__O2_BIT_ARR
#define HPP_OKIIDOKU__O2_BIT_ARR

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <array>
#include <bit>
#include <compare>
#include <cassert>

namespace okiidoku::mono {

	// Like `bitset` but with only what's needed, and a get index of nth
	// set bit operation with cool optimization.
	template<Order O> requires(is_order_compiled(O))
	struct O2BitArr final {
	public:
		using T = Ints<O>;
		using o2xs_t = int_ts::o2xs_t<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;

		using word_t = std::uint64_t;

		// Note: use of unsigned char is safe for grid-orders < 128. That should be fine.
		using word_i_t = unsigned char;
		using word_bit_i_t = unsigned char;

	private:
		static constexpr word_t word_t_num_bits {8 * sizeof(word_t)};
		static constexpr word_i_t num_words {(T::O2 + word_t_num_bits-1) / word_t_num_bits}; static_assert(num_words != 0);
		static constexpr word_bit_i_t num_excess_bits {(num_words * word_t_num_bits) - T::O2};

		[[nodiscard, gnu::const]]
		static constexpr word_i_t bit_i_to_word_i(const o2x_t bit_i) noexcept {
			return bit_i / word_t_num_bits;
		}

		// Internal Note: If user follows contracts, excess top bits are always zero.
		using words_t = std::array<word_t, num_words>;
		words_t words_ {word_t{0}};

	public:
		constexpr O2BitArr() noexcept = default;

		// Helper for `O2BitArr_ones`.
		static consteval O2BitArr create_ones_() noexcept {
			O2BitArr<O> _ {};
			for (auto& word : _.words_) { word = ~word; }
			_.words_.back() >>= O2BitArr<O>::num_excess_bits;
			return _;
		}

		// count the number of set bits.
		[[nodiscard, gnu::pure]] o2i_t count() const noexcept;
		// count the number of set bits below the specified bit index.
		// contract: `end < O2`.
		[[nodiscard, gnu::pure]] o2x_t count_set_bits_below(const o2x_t end) const noexcept;

		[[nodiscard, gnu::pure]] bool test(const o2x_t at) const noexcept {
			assert(at < T::O2);
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			return (words_[bit_i_to_word_i(at)] & word_bit_mask) != 0;
		}
		constexpr void set(const o2x_t at) noexcept {
			assert(at < T::O2);
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			words_[bit_i_to_word_i(at)] |= word_bit_mask;
		}
		constexpr void unset(const o2x_t at) noexcept {
			assert(at < T::O2);
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			words_[bit_i_to_word_i(at)] &= ~word_bit_mask;
		}

		void remove(const O2BitArr& to_remove) noexcept {
			for (word_i_t i {0}; i < num_words; ++i) {
				words_[i] &= ~to_remove.words_[i];
			}
		}
		void retain_only(const O2BitArr& to_retain) noexcept {
			for (word_i_t i {0}; i < num_words; ++i) {
				words_[i] &= to_retain.words_[i];
			}
		}

		void unset_all() noexcept {
			words_.fill(0);
		}

		[[nodiscard, gnu::pure]] static bool test_any3(const o2x_t at, const O2BitArr& a, const O2BitArr& b, const O2BitArr& c) noexcept {
			assert(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			return static_cast<bool>((a.words_[word_i] | b.words_[word_i] | c.words_[word_i]) & word_bit_mask);
		}
		static void set3(const o2x_t at, O2BitArr& a, O2BitArr& b, O2BitArr& c) noexcept {
			assert(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			a.words_[word_i] |= word_bit_mask;
			b.words_[word_i] |= word_bit_mask;
			c.words_[word_i] |= word_bit_mask;
		}
		static void unset3(const o2x_t at, O2BitArr& a, O2BitArr& b, O2BitArr& c) noexcept {
			assert(at < T::O2);
			const auto word_i {bit_i_to_word_i(at)};
			const word_t word_bit_mask {word_t{1} << (at % word_t_num_bits)};
			a.words_[word_i] &= ~word_bit_mask;
			b.words_[word_i] &= ~word_bit_mask;
			c.words_[word_i] &= ~word_bit_mask;
		}

		O2BitArr& operator|=(const O2BitArr& rhs) noexcept {
			for (word_i_t i {0}; i < num_words; ++i) { words_[i] |= rhs.words_[i]; };
			return *this;
		}
		O2BitArr& operator&=(const O2BitArr& rhs) noexcept {
			for (word_i_t i {0}; i < num_words; ++i) { words_[i] &= rhs.words_[i]; };
			return *this;
		}
		[[nodiscard, gnu::pure]] friend O2BitArr operator|(O2BitArr lhs, const O2BitArr& rhs) noexcept {
			lhs |= rhs; return lhs;
		}
		[[nodiscard, gnu::pure]] friend O2BitArr operator&(O2BitArr lhs, const O2BitArr& rhs) noexcept {
			lhs &= rhs; return lhs;
		}

		// contract: this mask has at least one set bit.
		// a suitably long and ugly name for a sharp, niche, optimized knife.
		[[nodiscard, gnu::pure]] o2xs_t count_lower_zeros_assuming_non_empty_mask() const noexcept;

		// contract: `set_bit_i` < O2 and there are at least `set_bit_i+1` set bits.
		[[nodiscard, gnu::pure]] o2x_t get_index_of_nth_set_bit(o2x_t set_bit_i) const noexcept;

		class SetBitsWalker final {
		public:
			explicit SetBitsWalker(O2BitArr arr) noexcept: arr_{arr} {
				advance();
			}
			[[nodiscard, gnu::pure]] bool has_more() const noexcept { return word_i < num_words; }
			[[nodiscard, gnu::pure]] o2x_t value() const noexcept {
				assert(has_more());
				return static_cast<o2x_t>((word_i * word_t_num_bits) + word_bit_i);
			}
			void advance() noexcept {
				assert(has_more());
				while (word_i < num_words && arr_.words_[word_i] == 0) { ++word_i; }
				if (has_more()) {
					auto& word {arr_.words_[word_i]};
					word_bit_i = static_cast<word_bit_i_t>(std::countr_zero(word));
					word &= static_cast<word_t>(word-1U); // unset lowest bit
				}
			}
		private:
			O2BitArr arr_;
			word_i_t word_i {0};
			word_bit_i_t word_bit_i {0};
		};
		[[nodiscard, gnu::pure]] SetBitsWalker set_bits_walker() const noexcept {
			return SetBitsWalker(*this);
		}

		// Defines a strong ordering between masks. Needs design and testing work.
		[[nodiscard, gnu::pure]] static std::strong_ordering cmp_differences(const O2BitArr& a, const O2BitArr& b) noexcept;

		// least-significant bit is the least-significant (left-most) character.
		[[nodiscard, gnu::pure]] std::array<char, T::O2> to_stringbuf() const noexcept;
	};
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		static_assert(!std::is_aggregate_v<O2BitArr<O_>>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O


	template<Order O> requires(is_order_compiled(O))
	static constexpr O2BitArr<O> O2BitArr_ones {O2BitArr<O>::create_ones_()};


	template<Order O>
	struct chute_box_masks final {
		using M = O2BitArr<O>;
		static constexpr std::array<M, O> row {[]{
			std::array<M, O> mask;
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					mask[chute].set(static_cast<int_ts::o2x_t<O>>((O*chute) + i));
			}	}
			return mask;
		}()};
		static constexpr std::array<M, O> col {[]{
			std::array<M, O> mask;
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					mask[chute].set(static_cast<int_ts::o2x_t<O>>((O*i) + chute));
			}	}
			return mask;
		}()};
	};
}
#endif