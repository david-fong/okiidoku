// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__MIXED_RADIX_INT_SERDES
#define HPP_OKIIDOKU__DETAIL__MIXED_RADIX_INT_SERDES

#include <iosfwd>
#include <concepts>
#include <utility>
#include <limits>   // numeric_limits, ...
#include <bit>      // byteswap, bit_width
#include <cstdint>
#include <climits>  // CHAR_BIT

namespace okiidoku::detail {

	template<class T>
	concept radix = std::numeric_limits<T>::is_specialized
		&&  std::numeric_limits<T>::is_integer
		&& !std::numeric_limits<T>::is_signed
		&&  std::numeric_limits<T>::round_style == std::round_toward_zero
		&&  std::numeric_limits<T>::max() > 1u
		;

	/** for writing a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned integer to a stream.
	\note this is not a generalized interface for mixed-radix integers. the reader
		interface is greedy to allow more-significant radices to be determined based on
		less-significant digit values- a property the Grid serdes facilities rely upon.
		otherwise, the implementation here could try to optimize layout/grouping of int
		for better packing. */
	template<radix RadixType_, std::unsigned_integral BufType_ = std::uintmax_t>
		requires(std::numeric_limits<RadixType_>::max()-1u <= std::numeric_limits<BufType_>::max()) // simplifies implementation
	class MixedRadixUintWriter {
	public:
		using radix_t = RadixType_;
		using buf_t = BufType_;
		struct [[gnu::designated_init]] Item {
			radix_t radix;
			radix_t digit;
		};
	private:
		static constexpr radix_t radix_t_max {std::numeric_limits<radix_t>::max()};
		static constexpr buf_t   buf_t_max   {std::numeric_limits<buf_t  >::max()};
			static_assert(radix_t_max-1u <= buf_t_max);
		static constexpr std::uint_fast8_t num_buf_bytes {sizeof(buf_t)};
		static constexpr std::uint_fast8_t num_buf_bits  {num_buf_bytes*CHAR_BIT};

		std::array<Item, num_buf_bits+1uz> queue_; // lower indices are for less-significant radix/digit positions
		std::uint_fast8_t queue_end_ {0u};
		bool did_overflow_ {false};
		buf_t buf_meter_ {0u};
		std::size_t bytes_written_ {0uz};
		// buf = q[0].d + q[1].d*q[0].r + q[2].d*q[1].r*q[0].r + q[3].d*q[2].r*q[1].r*q[0].r
		// buf += q[3].d; buf *= q[2].r; buf += q[2].d; buf *= q[1].r; buf += q[1].d; buf *= q[0].r; buf += q[0].d;

	public:
		/** accept a digit for writing.
		call this once for each digit of the integer, from least to most significant.
		\return `true` if a `flush` isn't needed yet; `false` if a `flush` is now needed.
		\pre if this is the `n`th call, it is for the `n`th least-significant digit.
		\pre `digit < radix`.
		\pre `radix > 0`.
		\pre the previous call didn't indicate a need for a `flush`. */
		// TODO try making this take a `Item` and see if callers can use designated init-list
		[[nodiscard]] bool accept(radix_t radix, radix_t digit) noexcept {
			OKIIDOKU_CONTRACT_USE(!did_overflow_);
			OKIIDOKU_CONTRACT_USE(queue_end_ <= queue_.size());
			OKIIDOKU_CONTRACT_USE(radix > 0u);
			OKIIDOKU_CONTRACT_USE(digit < radix);
			if (radix < 2u) [[unlikely]] { return true; }
			if ((buf_meter_ > buf_t_max/radix) || ((radix-1u) > buf_t_max-(buf_meter_*radix))) [[unlikely]] {
				// handle overflow
				OKIIDOKU_CONTRACT_USE(buf_meter_ != 0u);
				// try to use remaining space in buf:
				const auto radix_0 {static_cast<radix_t>(buf_t_max / buf_meter_)};
				if (radix_0 <= 2u) {
					OKIIDOKU_CONTRACT_USE(radix_0 < radix);
					buf_meter_ = (buf_meter_*radix_0)+(radix_0-1u);
					auto& qi {queue_[queue_end_]};
					qi.radix = radix_0;
					qi.digit = digit % radix_0;
					++queue_end_;
					radix /= radix_0;
					digit /= radix_0;
				}
				// save overflow:
				auto& qi {queue_[queue_end_]};
				qi.radix = radix;
				qi.digit = digit;
				did_overflow_ = true;
				return false;
			}
			buf_meter_ = (buf_meter_*radix)+(radix-1u);
			auto& qi {queue_[queue_end_]}; // TODO try to rewrite these 4 lines and similar above with post-increment and copy-assignment with designated init-list.
			qi.radix = radix;
			qi.digit = digit;
			++queue_end_;
			return buf_meter_ < buf_t_max; // or more efficiently(?), false if buf_t_max/buf_meter_ >= 2
		}

		/**
		call this exactly once when the previous call to `accept` indicated a need to flush,
		or there is no more data to `accept`.
		\pre the previous call to `accept` indicated a need to flush, or there is no more data. */
		void flush(std::ostream& os) {
			if (queue_end_ == 0u) [[unlikely]] { return; } // short-circuit. technically unnecessary.
			buf_t buf {0u};
			for (auto i {0uz}; i < queue_end_; ++i) {
				const auto& qi {queue_[queue_end_-1uz-i]};
				buf *= qi.radix;
				buf += qi.digit;
			}
			const auto num_bytes {static_cast<std::uint_fast8_t>(std::bit_width(buf_meter_) / CHAR_BIT)};
			OKIIDOKU_CONTRACT_USE(num_bytes <= num_buf_bytes);
			if (num_bytes < num_buf_bytes) [[unlikely]] {
				bytes_written_ += num_bytes;
				for (auto i {0uz}; i < num_bytes; ++i) {
					os.put(static_cast<char>(buf));
					buf >>= CHAR_BIT;
				}
			} else {
				bytes_written_ += num_buf_bytes;
				if constexpr (std::endian::native == std::endian::little) {
					os.write(reinterpret_cast<char*>(&buf), num_buf_bytes);
				} else {
					for (auto i {0uz}; i < num_buf_bytes; ++i) {
						os.put(static_cast<char>(buf));
						buf >>= CHAR_BIT;
					}
				}
			}
			if (did_overflow_) [[likely]] {
				did_overflow_ = false;
				queue_[0uz] = queue_[queue_end_];
				queue_end_ = 1u;
			}
		}

		[[nodiscard, gnu::pure]] std::size_t bytes_written() noexcept {
			return bytes_written_;
		}
	};


	/** for reading a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned integer from a stream. */
	template<radix RadixType_, std::unsigned_integral BufType_ = std::uintmax_t>
	class MixedRadixUintReader {
	public:
		using radix_t = RadixType_;
		using buf_t = BufType_;
	private:
		static constexpr buf_t buf_t_max {std::numeric_limits<buf_t>::max()};
		static constexpr std::uint_fast8_t num_buf_bytes {sizeof(buf_t)};
		static constexpr std::uint_fast8_t num_buf_bits {num_buf_bytes*CHAR_BIT};
		buf_t buf_ {0u};
		buf_t buf_meter_ {0u};
		std::size_t bytes_read_ {0uz};

		void read_buf(std::istream& is) {
			buf_meter_ = buf_t_max;
			bytes_read_ += num_buf_bytes;
			if constexpr (std::endian::native == std::endian::little) {
				is.read(reinterpret_cast<char*>(&buf_), num_buf_bytes);
			} else if constexpr (std::endian::native == std::endian::little) {
				is.read(reinterpret_cast<char*>(&buf_), num_buf_bytes);
				std::byteswap(buf_);
			} else {
				buf_ = 0u;
				for (auto i {0uz}; i < num_buf_bytes; ++i) {
					char c;
					is.get(c);
					buf_ |= buf_t{c} << (i*CHAR_BIT);
				}
			}
		}

	public:
		/**
		\pre the data being read was written by a writer with the same type parameters.
		\pre if this is the `n`th call, it is for the `n`th least-significant digit, and
			`radix` is the same value that was passed to write this digit.
		\pre `is` opened with same text/binary mode used when writing this data.
		\pre `radix > 0`.
		*/
		[[nodiscard]] radix_t read(std::istream& is, buf_t radix, const bool is_last) {
			OKIIDOKU_CONTRACT_USE(radix > 0u);
			if (radix < 2u) [[unlikely]] { return 0u; }
			if (buf_meter_ == 0u) [[unlikely]] {
				read_buf(is);
			}
			radix_t digit {0u};
			if (buf_meter_ < radix-1u) [[unlikely]] {
				digit = static_cast<radix_t>(buf_);
				radix /= static_cast<radix_t>(buf_meter_)+1u;
				read_buf(is);
			}
			OKIIDOKU_CONTRACT_USE(buf_meter_ >= radix-1u);
			digit = static_cast<radix_t>(buf_ % radix);
			buf_ /= radix;
			buf_meter_ /= radix;
			if (is_last) [[unlikely]] {
				is.seekg(-(std::bit_width(buf_meter_) / CHAR_BIT), std::ios_base::cur);
			}
			return digit;
		}

		/** \pre there is no more data to read. */
		[[nodiscard, gnu::pure]] std::size_t bytes_read() noexcept {
			return bytes_read_;
		}
	};
}
#endif