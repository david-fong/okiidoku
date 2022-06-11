#include <okiidoku/o2_bit_arr.hpp>

#include <numeric> // transform_reduce
#include <execution>
#include <cassert>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2i_t
	O2BitArr<O>::count() const noexcept {
		if constexpr (num_words == 1) {
			const auto count {std::popcount(words_[0])};
			OKIIDOKU_CONTRACT_TRIVIAL_EVAL(count <= T::O2);
			return static_cast<o2i_t>(count);
		} else {
			const auto count {std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				words_.cbegin(), words_.cend(), static_cast<o2i_t>(0U), std::plus<o2i_t>{},
				[](const auto& word){ return static_cast<o2i_t>(std::popcount(word)); }
			)};
			OKIIDOKU_CONTRACT_TRIVIAL_EVAL(count <= T::O2);
			return static_cast<o2i_t>(count);
		}
	}


	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2x_t
	O2BitArr<O>::count_set_bits_below(const typename O2BitArr<O>::o2x_t end) const noexcept {
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(end < T::O2);
		const auto end_at_int {bit_i_to_word_i(end)};
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(end_at_int < num_words);
		return static_cast<o2x_t>(std::transform_reduce(
			#ifdef __cpp_lib_execution
			std::execution::unseq,
			#endif
			words_.cbegin(), std::next(words_.cbegin(), end_at_int),
			/* init value: */static_cast<o2x_t>(std::popcount(static_cast<word_t>(
				words_[end_at_int] & (word_bit_mask_for_bit_i(end) - static_cast<word_t>(1U))
			))),
			std::plus<o2x_t>{},
			[](const auto& word){ return static_cast<o2x_t>(std::popcount(word)); }
		));
	}


	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2xs_t
	O2BitArr<O>::count_lower_zeros_assuming_non_empty_mask() const noexcept {
		// Note: without the non-empty-mask assumption, we'd have to
		//  handle discounting excess top zeros in the empty-mask case.
		assert(count() > 0);
		if constexpr (num_words == 1) {
			return static_cast<o2xs_t>(std::countr_zero(words_[0]));
		} else {
			o2xs_t count {0};
			for (const auto& word : words_) {
				if (word == 0) {
					count += word_t_num_bits;
				} else {
					count += static_cast<o2xs_t>(std::countr_zero(word));
					break;
				}
			}
			return count;
		}
	}


	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2x_t
	O2BitArr<O>::get_index_of_nth_set_bit(O2BitArr::o2x_t set_bit_index) const noexcept {
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(set_bit_index < T::O2);
		assert(count() > o2i_t{set_bit_index});
		for (word_i_t word_i {0}; word_i < num_words; ++word_i) {
			auto& word {words_[word_i]};
			const auto word_popcount {std::popcount(word)};
			if constexpr (num_words > 1) {
				if (static_cast<o2x_t>(word_popcount) <= set_bit_index) [[likely]] {
					set_bit_index -= static_cast<o2x_t>(word_popcount);
					continue;
				}
			}
			for (word_t word_bit_i {0}; word_bit_i < word_t_num_bits; ++word_bit_i) { // TODO.mid possible optimization: skip consecutive set bits by somehow using std::countr_<>
				const auto bit_mask {word_t{1} << word_bit_i};
				if (word & bit_mask) {
					if (set_bit_index == 0) {
						// word &= ~bit_mask;
						return static_cast<o2x_t>((word_t_num_bits * word_i) + word_bit_i);
					}
					--set_bit_index;
				}
			}
			// TODO.high if has pdep instruction:
			// const auto bit_mask {_pdep_u64(1 << set_bit_index, word)};
			//// word &= ~bit_mask;
			// return std::countr_zero(bit_mask);
		}
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(false); // TODO.wait c++23 std::unreachable
		return 0;
	}


	template<Order O> requires(is_order_compiled(O))
	std::strong_ordering
	O2BitArr<O>::cmp_differences(const O2BitArr<O>& a, const O2BitArr<O>& b) noexcept {
		for (word_i_t i {0}; i < num_words; ++i) {
			const auto diffs {a.words_[i] ^ b.words_[i]};
			if (const auto cmp {(a.words_[i]&diffs) <=> (b.words_[i]&diffs)}; std::is_neq(cmp)) [[likely]] { return cmp; }
		}
		return std::strong_ordering::equivalent;
	}


	template<Order O> requires(is_order_compiled(O))
	std::array<char, Ints<O>::O2>
	O2BitArr<O>::to_stringbuf() const noexcept {
		std::array<char, T::O2> _; // NOLINT(cppcoreguidelines-pro-type-member-init) see next line
		_.fill('.');
		for (o2i_t i {0}; i < T::O2; ++i) {
			if (test(static_cast<o2x_t>(i))) { _[i] = '1'; }
		}
		return _;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template typename O2BitArr<O_>::o2i_t O2BitArr<O_>::count() const noexcept; \
		template typename O2BitArr<O_>::o2x_t O2BitArr<O_>::count_set_bits_below(typename O2BitArr<O_>::o2x_t) const noexcept; \
		template typename O2BitArr<O_>::o2xs_t O2BitArr<O_>::count_lower_zeros_assuming_non_empty_mask() const noexcept; \
		template typename O2BitArr<O_>::o2xs_t O2BitArr<O_>::get_index_of_nth_set_bit(O2BitArr<O_>::o2x_t) const noexcept; \
		template std::strong_ordering O2BitArr<O_>::cmp_differences(const O2BitArr<O_>&, const O2BitArr<O_>&) noexcept; \
		template std::array<char, Ints<O_>::O2> O2BitArr<O_>::to_stringbuf() const noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}