// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/o2_bit_arr.hpp>

#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif

#include <algorithm> // find_if
#include <numeric> // transform_reduce
#include <execution>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2i_t
	O2BitArr<O>::count() const noexcept {
		if constexpr (num_words == 1) {
			const auto count {static_cast<o2i_t>(std::popcount(words_[0]))};
			OKIIDOKU_CONTRACT_USE(count <= T::O2);
			return count;
		} else {
			const auto count {static_cast<o2i_t>(std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				words_.cbegin(), words_.cend(), o2i_t{0}, std::plus<o2i_t>{},
				[](const auto& word){ return static_cast<o2i_t>(std::popcount(word)); }
			))};
			OKIIDOKU_CONTRACT_USE(count <= T::O2);
			return count;
		}
	}


	// TODO compiler having trouble inferring ability to use bzhi instruction for O=3,4 when they use u16 instead of u32.
	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2x_t
	O2BitArr<O>::count_below(const typename O2BitArr<O>::o2x_t end) const noexcept {
		OKIIDOKU_CONTRACT_USE(end < T::O2);
		if constexpr (num_words == 1) {
			return static_cast<o2x_t>(std::popcount(static_cast<word_t>(
				words_[0] & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1})
			)));
		} else {
			const auto end_at_int {bit_i_to_word_i(end)};
			OKIIDOKU_CONTRACT_USE(end_at_int < num_words);
			return static_cast<o2x_t>(std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				words_.cbegin(), std::next(words_.cbegin(), end_at_int),
				/* init value: */static_cast<o2x_t>(std::popcount(static_cast<word_t>(
					words_[end_at_int] & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1})
				))),
				std::plus<o2x_t>{},
				[](const auto& word){ return static_cast<o2x_t>(std::popcount(word)); }
			));
		}
	}


	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2xs_t
	O2BitArr<O>::first_set_bit_require_exists() const noexcept {
		// Note: without the non-empty-mask assumption, we'd have to
		//  handle discounting excess top zeros in the empty-mask case.
		OKIIDOKU_CONTRACT_USE(count() > 0);
		if constexpr (num_words == 1) {
			const auto count {static_cast<o2xs_t>(std::countr_zero(words_[0]))};
			OKIIDOKU_CONTRACT_USE(count < T::O2);
			return count;
		} else {
			const auto word {std::find_if(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				words_.cbegin(), words_.cend(), [](const auto& w){ return w != 0; }
			)};
			const o2xs_t count {static_cast<o2x_t>(
				/* static_cast<o2x_t> */(word_t_num_bits*static_cast<o2x_t>(std::distance(words_.cbegin(), word)))
				+ static_cast<o2x_t>(std::countr_zero(*word))
			)};
			OKIIDOKU_CONTRACT_USE(count < T::O2);
			return count;
		}
	}


	template<Order O> requires(is_order_compiled(O))
	typename O2BitArr<O>::o2x_t
	O2BitArr<O>::get_index_of_nth_set_bit(O2BitArr::o2x_t set_bit_index) const noexcept {
		OKIIDOKU_CONTRACT_USE(set_bit_index < T::O2);
		OKIIDOKU_CONTRACT_USE(count() > set_bit_index);
		const word_i_t word_i {[&](){
			if constexpr (num_words == 1) { return word_i_t{0}; }
			else {
				for (word_i_t wd_i {0}; wd_i < num_words; ++wd_i) {
					const auto& word {words_[wd_i]};
					const auto word_popcount {static_cast<o2i_t>(std::popcount(word))};
					OKIIDOKU_CONTRACT_USE(word_popcount <= word_t_num_bits);
					if (set_bit_index >= word_popcount) [[likely]] {
						set_bit_index = static_cast<o2x_t>(set_bit_index - word_popcount);
					} else {
						return wd_i;
					}
				}
				OKIIDOKU_UNREACHABLE;
			}
		}()};
		const auto& word {words_[word_i]};
		#ifdef OKIIDOKU_TARGET_SUPPORTS_X86_BMI2
			if constexpr (sizeof(word_t) >= sizeof(std::uint64_t)) {
				const auto bit_mask {_pdep_u64(static_cast<word_t>(word_t{1} << set_bit_index), word)};
				return static_cast<o2x_t>(static_cast<o2x_t>(word_t_num_bits*word_i) + std::countr_zero(bit_mask));
			} else {
				const auto bit_mask {_pdep_u32(static_cast<word_t>(word_t{1} << set_bit_index), word)};
				return static_cast<o2x_t>(static_cast<o2x_t>(word_t_num_bits*word_i) + std::countr_zero(bit_mask));
			}
		#else
		for (word_t word_bit_i {0}; word_bit_i < word_t_num_bits; ++word_bit_i) { // TODO.mid possible optimization: skip consecutive set bits by somehow using std::countr_<>
			const auto bit_mask {static_cast<word_t>(word_t{1} << word_bit_i)};
			if (word & bit_mask) {
				if (set_bit_index == 0) {
					// word &= ~bit_mask;
					return static_cast<o2x_t>((word_t_num_bits * word_i) + word_bit_i);
				}
				--set_bit_index;
			}
		}
		OKIIDOKU_UNREACHABLE;
		#endif
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
	O2BitArr<O>::to_chars() const noexcept {
		OKIIDOKU_NO_PRE_INIT_AUTOVAR std::array<char, T::O2> _; // NOLINT(cppcoreguidelines-pro-type-member-init) see next line
		_.fill('.');
		for (o2i_t i {0}; i < T::O2; ++i) {
			if (test(static_cast<o2x_t>(i))) { _[i] = '1'; }
		}
		return _;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template typename O2BitArr<O_>::o2i_t O2BitArr<O_>::count() const noexcept; \
		template typename O2BitArr<O_>::o2x_t O2BitArr<O_>::count_below(typename O2BitArr<O_>::o2x_t) const noexcept; \
		template typename O2BitArr<O_>::o2xs_t O2BitArr<O_>::first_set_bit_require_exists() const noexcept; \
		template typename O2BitArr<O_>::o2xs_t O2BitArr<O_>::get_index_of_nth_set_bit(O2BitArr<O_>::o2x_t) const noexcept; \
		template std::strong_ordering O2BitArr<O_>::cmp_differences(const O2BitArr<O_>&, const O2BitArr<O_>&) noexcept; \
		template std::array<char, Ints<O_>::O2> O2BitArr<O_>::to_chars() const noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}