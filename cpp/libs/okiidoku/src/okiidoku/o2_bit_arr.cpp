// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/o2_bit_arr.hpp>

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif

#include <algorithm>   // find_if
#include <functional>  // plus
#include <numeric>     // transform_reduce
#include <execution>
#include <type_traits> // is_aggregate_v

namespace okiidoku::mono {

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		static_assert(!std::is_aggregate_v<BitArray<(O_)*(O_)>>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT

	template<std::uintmax_t width_, IntKind kind_>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	auto BitArray<width_,kind_>::count() const noexcept -> typename BitArray::bit_ii_t {
		if constexpr (num_words == 1u) {
			/*RVO*/bit_ii_t count {std::popcount(words_.front())};
			count.check();
			return count;
		} else {
			/*RVO*/bit_ii_t count {std::transform_reduce(OKIIDOKU_UNSEQ
				words_.cbegin(), words_.cend(), bit_ii_t{0u}, std::plus<bit_ii_t>{},
				[][[gnu::const]](const auto word)noexcept{ return bit_ii_t{std::popcount(word)}; }
			)};
			count.check();
			return count;
		}
	}


	// TODO compiler having trouble inferring ability to use bzhi instruction for O=3,4 when they use u16 instead of u32.
	template<std::uintmax_t width_, IntKind kind_>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	auto BitArray<width_,kind_>::count_below(const typename BitArray::bit_ix_t end) const noexcept -> typename BitArray::bit_ix_t {
		end.check();
		if constexpr (num_words == 1u) {
			return bit_ix_t{std::popcount(
				static_cast<word_t>(words_.front() & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1u}))
			)};
		} else {
			const auto end_at_int {bit_i_to_word_i(end)};
			OKIIDOKU_CONTRACT(end_at_int < num_words);
			return bit_ix_t{std::transform_reduce(OKIIDOKU_UNSEQ
				words_.cbegin(), std::next(words_.cbegin(), end_at_int),
				/* init value: */bit_ix_t{std::popcount(
					words_[end_at_int] & static_cast<word_t>(word_bit_mask_for_bit_i(end) - word_t{1u})
				)},
				std::plus<bit_ix_t>{},
				[][[gnu::const]](const auto word)noexcept{ return bit_ix_t{std::popcount(word)}; }
			)};
		}
	}


	template<std::uintmax_t width_, IntKind kind_>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	auto BitArray<width_,kind_>::first_set_bit() const noexcept -> typename BitArray::bit_ii_t {
		OKIIDOKU_ASSERT(count() > 0u);
		if constexpr (num_words == 1u) {
			if (words_.front() == 0u) [[unlikely]] { return width; }
			const bit_ii_t count {std::countr_zero(words_.front())};
			OKIIDOKU_CONTRACT(count < width);
			return count;
		} else {
			const auto word_it {std::find_if(OKIIDOKU_UNSEQ
				words_.cbegin(), words_.cend(), [][[gnu::const]](const auto w)noexcept{ return w != 0u; }
			)};
			if (word_it == words_.cend()) [[unlikely]] { return width; }
			/*RVO*/bit_ii_t count {
				(num_word_bits * std::distance(words_.cbegin(), word_it))
				+ std::countr_zero(*word_it)
			};
			OKIIDOKU_CONTRACT(count < width);
			return count;
		}
	}


	template<std::uintmax_t width_, IntKind kind_>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	auto BitArray<width_,kind_>::get_index_of_nth_set_bit(BitArray::bit_ix_t set_bit_index) const noexcept -> typename BitArray::bit_ix_t {
		set_bit_index.check();
		OKIIDOKU_ASSERT(count() > set_bit_index);
		const word_ix_t word_i {[&](){
			if constexpr (num_words == 1u) { return word_ix_t{0u}; }
			else {
				for (const auto wd_i : num_words) {
					const word_bit_ii_t wd_popcount {std::popcount(words_[wd_i])};
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
		if constexpr (OKIIDOKU_TARGET_SUPPORTS_X86_BMI2 && sizeof(word_t) == sizeof(std::uint64_t)) {
			const auto bit_mask {_pdep_u64(static_cast<word_t>(word_t{1u} << set_bit_index), word)};
			return (num_word_bits*word_i) + std::countr_zero(bit_mask);
		} else if constexpr (OKIIDOKU_TARGET_SUPPORTS_X86_BMI2 && sizeof(word_t) == sizeof(std::uint32_t)) {
			const auto bit_mask {_pdep_u32(static_cast<word_t>(word_t{1u} << set_bit_index), word)};
			return (num_word_bits*word_i) + std::countr_zero(bit_mask);
		}
		for (const auto word_bit_i : num_word_bits) { // TODO.mid possible optimization: skip consecutive set bits by somehow using std::countr_<>
			const auto bit_mask {static_cast<word_t>(word_t{1u} << word_bit_i)};
			OKIIDOKU_CONTRACT(bit_mask != 0u);
			if (word & bit_mask) {
				if (set_bit_index == 0u) {
					// word &= ~bit_mask;
					return (num_word_bits * word_i) + word_bit_i;
				}
				--set_bit_index;
			}
		}
		OKIIDOKU_UNREACHABLE;
	}


	// auto operator<=>(const BitArray<O>& a, const BitArray<O>& b) noexcept -> std::strong_ordering { // NOLINT(*easily-swappable*)
	// 	// if one has a set bit earlier, then it's greater?
	// 	return std::strong_ordering::equivalent;
	// }


	template<std::uintmax_t width_, IntKind kind_>
		requires((width_ <= INTMAX_MAX) && (kind_ == IntKind::small || kind_ == IntKind::fast))
	auto BitArray<width_,kind_>::to_chars() const noexcept -> std::array<char,width_> {
		std::array<char,width_> _ OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
		_.fill('.');
		for (const auto i : set_bits()) {
			_[i] = '1';
		}
		return _;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template struct BitArray<(O_)*(O_), IntKind::small>; \
		template struct BitArray<(O_)*(O_), IntKind::fast >; \
		static_assert(std::input_iterator<BitArray<(O_)*(O_)>::Iter>);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}