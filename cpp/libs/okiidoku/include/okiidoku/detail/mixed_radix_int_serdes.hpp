// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__MIXED_RADIX_INT_SERDES
#define HPP_OKIIDOKU__DETAIL__MIXED_RADIX_INT_SERDES

#include <iosfwd>
#include <concepts>
#include <utility>
#include <cstdint>
#include <climits>  // CHAR_BIT

namespace okiidoku::detail {

	/** for writing a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned integer to a stream. */
	template<class RadixType, std::unsigned_integral BufType_ = std::uintmax_t>
	class MixedRadixUintWriter {
		static_assert(!std::numeric_limits<RadixType>::is_signed);
		using radix_t = RadixType_;
		using buf_t = BufType_;
		constexpr size_t num_buf_bytes {sizeof(buf_t)};
		constexpr size_t num_buf_bits {num_buf_bytes*CHAR_BIT};
		struct [[gnu::designated_init]] QueueItem {
			radix_t radix;
			radix_t digit;
		};
		std::array<QueueItem, num_buf_bits+(sizeof(radix_t)/sizeof(buf_t))> queue_;
		size_t queue_end_ {0u};
		buf_t queue_radix_prod_ {1u};

	public:
		/** accept a digit for writing.
		\return `true` if a flush isn't needed yet.
		\pre `digit < radix`.
		\pre `radix > 0`. */
		bool accept(radix_t digit, radix_t radix) noexcept {
			OKIIDOKU_CONTRACT_USE(queue_radix_prod_ > 0u);
			OKIIDOKU_CONTRACT_USE(queue_end_ < queue_.size());
			OKIIDOKU_CONTRACT_USE(radix > 0u);
			OKIIDOKU_CONTRACT_USE(digit < radix);
			bool needs_flush {false};
			while (radix >= 2u) {
				const buf_t radix_0 {std::numeric_limits<buf_t>::max() / queue_radix_prod_};
				if (radix_0 < radix) [[unlikely]] {
					// ^would overflow
					needs_flush = true;
					if (radix_0 >= 2u) {
						auto& qi {queue_[queue_end_]};
						qi.radix = radix_0;
						qi.digit = digit % radix_0;
						queue_radix_prod_ *= radix_0;
						OKIIDOKU_CONTRACT_USE(queue_radix_prod_ <= std::numeric_limits<buf_t>::max());
						++queue_end_;
						radix /= radix_0;
						digit /= radix_0;
					}
				}
			}
			return needs_flush;
		}

		/**
		*/
		void flush(std::ostream& os) noexcept {
			buf_t buf {0u};
			for (auto i {0uz}; i < queue_end_; ++i) {
				const auto& qi {queue_[queue_end_-1uz-i]};
				buf += qi.digit;

			}
			queue_[0uz] = queue[queue_end_];
			queue_end_ = 1uz;
			for (auto i {0uz}; i < sizeof(buf_t); ++i) {
				os << static_cast<unsigned char>{buf};
				buf >>= CHAR_BIT;
			}
		}
	};

	/** for reading a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned integer from a stream. */
	template<class BaseType, std::unsigned_integral BufType_ = std::uintmax_t>
	class MixedRadixUintReader {
		using base_t = BaseType_;
		using buf_t = BufType_;
		constexpr size_t num_buf_bytes {sizeof(buf_t)};
		constexpr size_t num_buf_bits {num_buf_bytes*CHAR_BIT};
		buf_t buf_;

	public:
		/**
		\pre `is` opened with same text/binary mode used when writing this data.
		\pre `base > 0`.
		*/
		base_t read(std::istream& is, const buf_t base, const bool is_last) noexcept {
			OKIIDOKU_CONTRACT_USE(base > 0u);
			if (base < 2u) { return 0u; }
		}
	};
}