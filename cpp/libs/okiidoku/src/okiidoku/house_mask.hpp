#ifndef HPP_OKIIDOKU__HOUSE_MASK
#define HPP_OKIIDOKU__HOUSE_MASK

#include <okiidoku/traits.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <numeric> // accumulate
#include <array>
#include <bit>
#include <cassert>

namespace okiidoku::mono {

	// Like `bitset` but with only what's needed, and a get index of nth
	// set bit operation with cool optimization.
	template<Order O> requires(is_order_compiled(O))
	class HouseMask final {
	private:
		using T = traits<O>;
		using o2x_t = typename traits<O>::o2x_t;
		using o2i_t = typename traits<O>::o2i_t;

		using int_t = std::uint64_t;

		static constexpr unsigned int int_t_num_bits  {8 * sizeof(int_t)};
		static constexpr unsigned int num_ints        {(T::O2 + int_t_num_bits-1) / int_t_num_bits};
		static constexpr unsigned int ints_t_num_bits {num_ints * int_t_num_bits};
		static constexpr unsigned int num_excess_bits {ints_t_num_bits - T::O2};
		[[nodiscard, gnu::const]] static constexpr unsigned char bit_index_to_int_index(const o2x_t bit_index) noexcept {
			return bit_index / int_t_num_bits;
		}
		using ints_t = std::array<int_t, num_ints>;

		ints_t ints_;

	public:
		constexpr HouseMask() noexcept: ints_{{0}} {}

		static constexpr HouseMask ones {[]{
			HouseMask _;
			for (auto& int_ : _.ints_) { int_ = ~int_; }
			_.ints_.back() >>= num_excess_bits;
			return _;
		}()};

		[[nodiscard, gnu::pure]] o2i_t count() const noexcept {
			return static_cast<o2i_t>(std::transform_reduce(
				ints_.cbegin(), ints_.cend(), 0, std::plus{},
				[&](const auto int_){ return static_cast<o2i_t>(std::popcount(int_)); }
			));
		}
		[[nodiscard, gnu::pure]] o2i_t count_bits_below(const o2i_t top_exclusive) const noexcept {
			auto count {static_cast<o2i_t>(std::popcount(ints_[num_ints-1] & static_cast<o2i_t>(top_exclusive - 1u)))};
			if constexpr (num_ints == 0) { return count; }
			for (size_t i {0}; i < num_ints - 1; ++i) {
				count += static_cast<o2i_t>(std::popcount(ints_[i]));
			}
			return count;
		}

		[[nodiscard, gnu::pure]] bool test(const o2x_t at) const noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			return static_cast<bool>(ints_[at_int] & at_int_bit_mask);
		}
		constexpr void set(const o2x_t at) noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			ints_[at_int] |= at_int_bit_mask;
		}
		constexpr void unset(const o2x_t at) noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			ints_[at_int] &= ~at_int_bit_mask;
		}

		[[nodiscard, gnu::pure]] static bool test_any3(const o2x_t at, const HouseMask& a, const HouseMask& b, const HouseMask& c) noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			return static_cast<bool>((a.ints_[at_int] | b.ints_[at_int] | c.ints_[at_int]) & at_int_bit_mask);
		}
		static void set3(const o2x_t at, HouseMask& a, HouseMask& b, HouseMask& c) noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			a.ints_[at_int] |= at_int_bit_mask;
			b.ints_[at_int] |= at_int_bit_mask;
			c.ints_[at_int] |= at_int_bit_mask;
		}
		static void unset3(const o2x_t at, HouseMask& a, HouseMask& b, HouseMask& c) noexcept {
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			a.ints_[at_int] &= ~at_int_bit_mask;
			b.ints_[at_int] &= ~at_int_bit_mask;
			c.ints_[at_int] &= ~at_int_bit_mask;
		}

		HouseMask& operator|=(const HouseMask& rhs) noexcept {
			for (size_t i {0}; i < ints_.size(); ++i) { ints_[i] |= rhs.ints_[i]; }; return *this;
		}
		HouseMask& operator&=(const HouseMask& rhs) noexcept {
			for (size_t i {0}; i < ints_.size(); ++i) { ints_[i] &= rhs.ints_[i]; }; return *this;
		}
		[[nodiscard, gnu::pure]] friend HouseMask operator|(HouseMask lhs, const HouseMask& rhs) noexcept {
			lhs |= rhs; return lhs;
		}
		[[nodiscard, gnu::pure]] friend HouseMask operator&(HouseMask lhs, const HouseMask& rhs) noexcept {
			lhs &= rhs; return lhs;
		}

		// contract: there are at least `set_bit_index+1` set bits.
		[[nodiscard, gnu::pure]] o2x_t get_index_of_nth_set_bit_and_unset(o2x_t set_bit_index) const noexcept {
			assert(count() > o2i_t{set_bit_index});
			for (size_t int_i {0}; int_i < ints_.size(); ++int_i) {
				auto& int_ {ints_[int_i]};
				const auto int_popcount {std::popcount(int_)};
				if (int_popcount > set_bit_index) {
					for (unsigned bit_i {0}; bit_i < int_t_num_bits; ++bit_i) { // TODO.mid possible optimization: skip consecutive on bits by somehow using std::countr_<>
						const auto bit_mask {int_t{1} << bit_i};
						if (int_ & bit_mask) {
							if (set_bit_index == 0) {
								int_ &= ~bit_mask;
								return static_cast<o2x_t>((int_t_num_bits * int_i) + bit_i);
							}
							--set_bit_index;
						}
					}
					// TODO.high if has pdep instruction:
					// const auto bit_mask {_pdep_u64(1 << set_bit_index, int_)};
					// int_ &= ~bit_mask;
					// return std::countr_zero(bit_mask);
				} else {
					set_bit_index -= int_popcount;
				}
			}
			assert(false); // TODO.wait c++23 std::unreachable
			return 0; 
		}
	};
	

	template<Order O>
	struct chute_box_masks final {
		using M = HouseMask<O>;
		static constexpr std::array<M, O> row {[]{
			std::array<M, O> mask;
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					mask[chute].set(static_cast<typename traits<O>::o2x_t>((O*chute) + i));
			}	}
			return mask;
		}()};
		static constexpr std::array<M, O> col {[]{
			std::array<M, O> mask;
			for (unsigned chute {0}; chute < O; ++chute) {
				for (unsigned i {0}; i < O; ++i) {
					mask[chute].set(static_cast<typename traits<O>::o2x_t>((O*i) + chute));
			}	}
			return mask;
		}()};
	};
}
#endif