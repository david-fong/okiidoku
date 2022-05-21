#include <okiidoku/house_mask.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	typename HouseMask<O>::o2xs_t
	HouseMask<O>::count_lower_zeros_assuming_non_empty_mask() const noexcept {
		assert(count() > 0);
		o2xs_t count {0};
		for (const auto& int_ : ints_) {
			if (int_ == 0) {
				count += int_t_num_bits;
			} else {
				count += static_cast<o2xs_t>(std::countr_zero(int_));
				// Note: without the non-empty-mask assumption, we'd have to
				// handle discounting excess top zeros in the empty-mask case.
				break;
			}
		}
		return count;
	}


	template<Order O> requires(is_order_compiled(O))
	typename HouseMask<O>::o2x_t
	HouseMask<O>::get_index_of_nth_set_bit(HouseMask::o2x_t set_bit_index) const noexcept {
		assert(set_bit_index < T::O2);
		assert(count() > o2i_t{set_bit_index});
		for (std::size_t int_i {0}; int_i < num_ints; ++int_i) {
			auto& int_ {ints_[int_i]};
			const auto int_popcount {std::popcount(int_)};
			if constexpr (num_ints > 1) {
				if (static_cast<o2x_t>(int_popcount) <= set_bit_index) [[likely]] {
					set_bit_index -= static_cast<o2x_t>(int_popcount);
					continue;
				}
			}
			for (unsigned bit_i {0}; bit_i < int_t_num_bits; ++bit_i) { // TODO.mid possible optimization: skip consecutive set bits by somehow using std::countr_<>
				const auto bit_mask {int_t{1} << bit_i};
				if (int_ & bit_mask) {
					if (set_bit_index == 0) {
						// int_ &= ~bit_mask;
						return static_cast<o2x_t>((int_t_num_bits * int_i) + bit_i);
					}
					--set_bit_index;
				}
			}
			// TODO.high if has pdep instruction:
			// const auto bit_mask {_pdep_u64(1 << set_bit_index, int_)};
			//// int_ &= ~bit_mask;
			// return std::countr_zero(bit_mask);
		}
		assert(false); // TODO.wait c++23 std::unreachable
		return 0;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template typename HouseMask<O_>::o2xs_t HouseMask<O_>::count_lower_zeros_assuming_non_empty_mask() const noexcept; \
		template typename HouseMask<O_>::o2xs_t HouseMask<O_>::get_index_of_nth_set_bit(HouseMask<O_>::o2x_t set_bit_index) const noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}