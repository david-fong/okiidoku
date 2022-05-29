#ifndef HPP_OKIIDOKU__HOUSE_MASK
#define HPP_OKIIDOKU__HOUSE_MASK

#include <okiidoku/ints.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <numeric>   // accumulate
#include <array>
#include <bit>
#include <compare>
#include <cassert>

namespace okiidoku::mono {

	// Like `bitset` but with only what's needed, and a get index of nth
	// set bit operation with cool optimization.
	template<Order O> requires(is_order_compiled(O))
	struct HouseMask final {
	public:
		using T = Ints<O>;
		using o2xs_t = int_ts::o2xs_t<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;

		using int_t = std::uint64_t;

	private:
		static constexpr unsigned int int_t_num_bits  {8 * sizeof(int_t)};
		static constexpr unsigned int num_ints        {(T::O2 + int_t_num_bits-1) / int_t_num_bits}; static_assert(num_ints != 0);
		static constexpr unsigned int ints_t_num_bits {num_ints * int_t_num_bits};
		static constexpr unsigned int num_excess_bits {ints_t_num_bits - T::O2};
		[[nodiscard, gnu::const]] static constexpr unsigned char bit_index_to_int_index(const o2x_t bit_index) noexcept {
			return bit_index / int_t_num_bits;
		}
		using ints_t = std::array<int_t, num_ints>;

		// if user follows contracts, excess top bits are always zero; this
		// property must not be depended upon by users of this library.
		ints_t ints_ {{0}};

	public:
		constexpr HouseMask() noexcept = default;

		// Helper for `house_mask_ones`.
		static consteval HouseMask create_ones_() noexcept {
			HouseMask<O> _;
			for (auto& int_ : _.ints_) { int_ = ~int_; }
			_.ints_.back() >>= HouseMask<O>::num_excess_bits;
			return _;
		}

		// count the number of set bits.
		[[nodiscard, gnu::pure]] constexpr o2i_t count() const noexcept {
			// TODO.mid investigate how good compilers are at optimizing these member functions.
			return static_cast<o2i_t>(std::transform_reduce(
				ints_.cbegin(), ints_.cend(), static_cast<o2i_t>(0U), std::plus{},
				[](const auto& int_){ return static_cast<o2i_t>(std::popcount(int_)); }
			));
		}
		// count the number of set bits below the specified bit index.
		// contract: `end < O2`.
		[[nodiscard, gnu::pure]] o2x_t count_set_bits_below(const o2x_t end) const noexcept {
			assert(end < T::O2);
			const auto end_at_int {bit_index_to_int_index(end)};
			assert(end_at_int < num_ints);
			auto count {static_cast<o2x_t>(std::popcount(
				ints_[end_at_int] & static_cast<int_t>(
					(static_cast<int_t>(1U) << (end % int_t_num_bits))
					- static_cast<int_t>(1U)
				)
			))};
			for (std::size_t i {0}; i < end_at_int; ++i) {
				count += static_cast<o2x_t>(std::popcount(ints_[i]));
			}
			return count;
		}

		[[nodiscard, gnu::pure]] bool test(const o2x_t at) const noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			return (ints_[at_int] & at_int_bit_mask) != 0;
		}
		constexpr void set(const o2x_t at) noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			ints_[at_int] |= at_int_bit_mask;
		}
		constexpr void unset(const o2x_t at) noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			ints_[at_int] &= ~at_int_bit_mask;
		}

		void remove(const HouseMask& to_remove) noexcept {
			for (std::size_t i {0}; i < num_ints; ++i) {
				ints_[i] &= ~to_remove.ints_[i];
			}
		}
		void retain_only(const HouseMask& to_retain) noexcept {
			for (std::size_t i {0}; i < num_ints; ++i) {
				ints_[i] &= to_retain.ints_[i];
			}
		}

		void unset_all() noexcept {
			ints_.fill(0);
		}

		[[nodiscard, gnu::pure]] static bool test_any3(const o2x_t at, const HouseMask& a, const HouseMask& b, const HouseMask& c) noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			return static_cast<bool>((a.ints_[at_int] | b.ints_[at_int] | c.ints_[at_int]) & at_int_bit_mask);
		}
		static void set3(const o2x_t at, HouseMask& a, HouseMask& b, HouseMask& c) noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			a.ints_[at_int] |= at_int_bit_mask;
			b.ints_[at_int] |= at_int_bit_mask;
			c.ints_[at_int] |= at_int_bit_mask;
		}
		static void unset3(const o2x_t at, HouseMask& a, HouseMask& b, HouseMask& c) noexcept {
			assert(at < T::O2);
			const auto at_int {bit_index_to_int_index(at)};
			const int_t at_int_bit_mask {int_t{1} << (at % int_t_num_bits)};
			a.ints_[at_int] &= ~at_int_bit_mask;
			b.ints_[at_int] &= ~at_int_bit_mask;
			c.ints_[at_int] &= ~at_int_bit_mask;
		}

		HouseMask& operator|=(const HouseMask& rhs) noexcept {
			for (std::size_t i {0}; i < num_ints; ++i) { ints_[i] |= rhs.ints_[i]; };
			return *this;
		}
		HouseMask& operator&=(const HouseMask& rhs) noexcept {
			for (std::size_t i {0}; i < num_ints; ++i) { ints_[i] &= rhs.ints_[i]; };
			return *this;
		}
		[[nodiscard, gnu::pure]] friend HouseMask operator|(HouseMask lhs, const HouseMask& rhs) noexcept {
			lhs |= rhs; return lhs;
		}
		[[nodiscard, gnu::pure]] friend HouseMask operator&(HouseMask lhs, const HouseMask& rhs) noexcept {
			lhs &= rhs; return lhs;
		}

		// contract: this mask has at least one set bit.
		// a suitably long and ugly name for a sharp, niche, optimized knife.
		[[nodiscard, gnu::pure]] o2xs_t count_lower_zeros_assuming_non_empty_mask() const noexcept;

		// contract: `set_bit_index` < O2 and there are at least `set_bit_index+1` set bits.
		[[nodiscard, gnu::pure]] o2x_t get_index_of_nth_set_bit(o2x_t set_bit_index) const noexcept;

		// Defines a strong ordering between masks. Its semantics are unspecified.
		// It is intended to be time-performant. Can be used to partition by masks
		// where the relationship between partitions doesn't matter.
		[[nodiscard, gnu::pure]] static std::strong_ordering cmp_differences(const HouseMask& a, const HouseMask& b) noexcept {
			for (std::size_t i {0}; i < num_ints; ++i) {
				const auto diffs {a.ints_[i] ^ b.ints_[i]};
				if (const auto cmp {(a.ints_[i]&diffs) <=> (b.ints_[i]&diffs)}; std::is_neq(cmp)) [[likely]] { return cmp; }
			}
			return std::strong_ordering::equivalent;
		}
	};
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		static_assert(!std::is_aggregate_v<HouseMask<O_>>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O


	template<Order O> requires(is_order_compiled(O))
	static constexpr HouseMask<O> house_mask_ones {HouseMask<O>::create_ones_()};


	template<Order O>
	struct chute_box_masks final {
		using M = HouseMask<O>;
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