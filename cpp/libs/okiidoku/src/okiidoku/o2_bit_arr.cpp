// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/o2_bit_arr.hpp>

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif

#include <algorithm>  // find_if
#include <functional> // plus
#include <numeric>    // transform_reduce
#include <execution>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::count() const noexcept -> typename O2BitArr<O>::o2i_t {
		if constexpr (num_words == 1u) {
			/*RVO*/o2i_t count {std::popcount(words_.front())};
			OKIIDOKU_CONTRACT(count <= T::O2);
			return count;
		} else {
			/*RVO*/o2i_t count {std::transform_reduce(
				OKIIDOKU_UNSEQ
				words_.cbegin(), words_.cend(), o2i_t{0u}, std::plus<o2i_t>{},
				[][[gnu::const]](const auto word)noexcept{ return o2i_t{std::popcount(word)}; }
			)};
			OKIIDOKU_CONTRACT(count <= T::O2);
			return count;
		}
	}


	// TODO compiler having trouble inferring ability to use bzhi instruction for O=3,4 when they use u16 instead of u32.
	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::count_below(const typename Ints<O>::o2x_t end) const noexcept -> typename Ints<O>::o2x_t {
		OKIIDOKU_CONTRACT(end < T::O2);
		if constexpr (num_words == 1u) {
			return o2x_t{std::popcount(
				words_.front() & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1u})
			)};
		} else {
			const auto end_at_int {bit_i_to_word_i(end)};
			OKIIDOKU_CONTRACT(end_at_int < num_words);
			return o2x_t{std::transform_reduce(
				OKIIDOKU_UNSEQ
				words_.cbegin(), std::next(words_.cbegin(), end_at_int),
				/* init value: */o2x_t{std::popcount(
					words_[end_at_int] & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1u})
				)},
				std::plus<o2x_t>{},
				[][[gnu::const]](const auto word)noexcept{ return o2x_t{std::popcount(word)}; }
			)};
		}
	}


	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::first_set_bit_require_exists() const noexcept -> typename O2BitArr<O>::o2xs_t {
		// Note: without the non-empty-mask assumption, we'd have to
		//  handle discounting excess top zeros in the empty-mask case.
		OKIIDOKU_ASSERT(count() > 0u);
		if constexpr (num_words == 1u) {
			const o2xs_t count {std::countr_zero(words_.front())};
			OKIIDOKU_CONTRACT(count < T::O2);
			return count;
		} else {
			const auto word {std::find_if(
				OKIIDOKU_UNSEQ
				words_.cbegin(), words_.cend(), [][[gnu::const]](const auto w)noexcept{ return w != 0u; }
			)};
			/*RVO*/o2xs_t count {
				(word_t_num_bits * std::distance(words_.cbegin(), word))
				+ std::countr_zero(*word)
			};
			OKIIDOKU_CONTRACT(count < T::O2);
			return count;
		}
	}


	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::get_index_of_nth_set_bit(O2BitArr::o2x_t set_bit_index) const noexcept -> typename O2BitArr<O>::o2x_t {
		set_bit_index.check();
		OKIIDOKU_ASSERT(count() > set_bit_index);
		const word_ix_t word_i {[&](){
			if constexpr (num_words == 1u) { return word_ix_t{0u}; }
			else {
				for (const auto wd_i : num_words) {
					const word_bit_i_t wd_popcount {std::popcount(words_[wd_i])};
					wd_popcount.check();
					if (set_bit_index >= wd_popcount) [[likely]] {
						set_bit_index -= wd_popcount;
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
				const auto bit_mask {_pdep_u64(static_cast<word_t>(word_t{1u} << set_bit_index), word)};
				return (word_t_num_bits*word_i) + std::countr_zero(bit_mask);
			} else {
				const auto bit_mask {_pdep_u32(static_cast<word_t>(word_t{1u} << set_bit_index), word)};
				return (word_t_num_bits*word_i) + std::countr_zero(bit_mask);
			}
		#else
		for (const auto word_bit_i : word_t_num_bits) { // TODO.mid possible optimization: skip consecutive set bits by somehow using std::countr_<>
			const auto bit_mask {static_cast<word_t>(word_t{1u} << word_bit_i)};
			if (word & bit_mask) {
				if (set_bit_index == 0u) {
					// word &= ~bit_mask;
					return (word_t_num_bits * word_i) + word_bit_i;
				}
				--set_bit_index;
			}
		}
		OKIIDOKU_UNREACHABLE;
		#endif
	}


	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::cmp_differences(const O2BitArr<O>& a, const O2BitArr<O>& b) noexcept -> std::strong_ordering { // NOLINT(*easily-swappable*)
		for (const auto i : num_words) {
			const auto diffs {a.words_[i] ^ b.words_[i]};
			if (const auto cmp {(a.words_[i]&diffs) <=> (b.words_[i]&diffs)}; std::is_neq(cmp)) [[likely]] { return cmp; }
		}
		return std::strong_ordering::equivalent;
	}


	template<Order O> requires(is_order_compiled(O))
	auto O2BitArr<O>::to_chars() const noexcept -> std::array<char, Ints<O>::O2> {
		OKIIDOKU_DEFER_INIT std::array<char, T::O2> _; // NOLINT(*-init)
		_.fill('.');
		for (const auto i : set_bits()) {
			_[i] = '1';
		}
		return _;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(std::input_iterator<O2BitArr<(O_)>::Iter>); \
		template typename O2BitArr<(O_)>::o2i_t  O2BitArr<(O_)>::count() const noexcept; \
		template typename O2BitArr<(O_)>::o2x_t  O2BitArr<(O_)>::count_below(typename O2BitArr<(O_)>::o2x_t) const noexcept; \
		template typename O2BitArr<(O_)>::o2xs_t O2BitArr<(O_)>::first_set_bit_require_exists() const noexcept; \
		template typename O2BitArr<(O_)>::o2x_t  O2BitArr<(O_)>::get_index_of_nth_set_bit(O2BitArr<(O_)>::o2x_t) const noexcept; \
		template std::strong_ordering O2BitArr<(O_)>::cmp_differences(const O2BitArr<(O_)>&, const O2BitArr<(O_)>&) noexcept; \
		template std::array<char, Ints<(O_)>::O2> O2BitArr<(O_)>::to_chars() const noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}