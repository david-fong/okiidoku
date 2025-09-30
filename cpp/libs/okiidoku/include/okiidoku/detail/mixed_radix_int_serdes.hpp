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
		requires(std::numeric_limits<RadixType_>::max()-1u <= std::numeric_limits<BufType_>::max())
		// ^this greatly simplifies implementation. if this restriction is changed, overflow handling and int casts will need to change.
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
		std::size_t digits_written_ {0uz};
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
			OKIIDOKU_CONTRACT_USE(radix <= radix_t_max);
			OKIIDOKU_CONTRACT_USE(radix > 0u);
			OKIIDOKU_CONTRACT_USE(digit < radix);
			++digits_written_;
			if (radix < 2u) [[unlikely]] { OKIIDOKU_CONTRACT_ASSERT(digit == 0u); return true; }
			if ((buf_meter_ > buf_t{buf_t_max/radix}) || (buf_t{radix-1u} > buf_t_max-buf_t{buf_meter_*radix})) [[unlikely]] {
				// handle overflow of `flush` buffer
				OKIIDOKU_CONTRACT_USE(buf_meter_ != 0u);
				// try to use remaining space in buf:
				const auto radix_0 {static_cast<radix_t>(buf_t_max / buf_meter_)};
				OKIIDOKU_CONTRACT_USE(radix_0 <= radix_t_max);
				OKIIDOKU_CONTRACT_USE(radix_0 < radix);
				if (radix_0 >= 2u) [[likely]] {
					buf_meter_ = (buf_meter_*radix_0)+(radix_0-1u);
					OKIIDOKU_CONTRACT_USE(queue_end_ < queue_.size()-1uz);
					auto& qi {queue_[queue_end_]};
					qi.radix = radix_0;
					qi.digit = std::min(digit, static_cast<radix_t>(radix_0 - radix_t{1u}));
					++queue_end_;
					radix = static_cast<radix_t>((radix-radix_0)+1u);
					digit -= qi.digit;
				}
				OKIIDOKU_CONTRACT_USE(radix >= 2u);
				OKIIDOKU_CONTRACT_USE(digit < radix);
				// save overflow and return:
				/* don't touch `buf_meter_`. `flush` uses non-overflowed value to determine
				number of bytes to write and otherwise doesn't care about it, and will
				reset it if overflow happened. */
				OKIIDOKU_CONTRACT_USE(queue_end_ < queue_.size());
				auto& qi {queue_[queue_end_]};
				qi.radix = radix;
				qi.digit = digit;
				did_overflow_ = true;
				return false;
			}
			buf_meter_ = buf_t{buf_meter_*radix}+buf_t{radix-1u};
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
			OKIIDOKU_CONTRACT_USE(queue_end_ > 0u);

			// prepare the real write buffer:
			buf_t buf {0u};
			for (auto i {0uz}; i < queue_end_; ++i) {
				const auto& qi {queue_[queue_end_-1uz-i]};
				OKIIDOKU_CONTRACT_USE(qi.radix > 0u);
				OKIIDOKU_CONTRACT_USE(qi.digit < qi.radix);
				buf *= qi.radix;
				buf += qi.digit;
			}
			OKIIDOKU_CONTRACT_USE(buf <= buf_meter_);

			// write `buf` to the stream:
			const auto num_bytes {static_cast<std::uint_fast8_t>((std::bit_width(buf_meter_)+(CHAR_BIT-1)) / CHAR_BIT)};
			OKIIDOKU_CONTRACT_USE(num_bytes <= num_buf_bytes);
			OKIIDOKU_CONTRACT_USE(num_bytes > 0u);
			bytes_written_ += num_bytes;
			if constexpr (std::endian::native == std::endian::little) {
				os.write(reinterpret_cast<char*>(&buf), num_bytes);
			} else {
				for (auto i {0uz}; i < num_bytes; ++i) {
					os.put(static_cast<char>(buf));
					buf >>= CHAR_BIT;
				}
			}

			if (did_overflow_) [[likely]] {
				did_overflow_ = false;
				queue_[0uz] = queue_[queue_end_];
				queue_end_ = 1u;
				const auto& qi {queue_[0uz]};
				OKIIDOKU_CONTRACT_USE(qi.radix > 0u);
				OKIIDOKU_CONTRACT_USE(qi.digit < qi.radix);
				buf_meter_ = qi.radix-1u;
			}
		}

		[[nodiscard, gnu::pure]] std::size_t digits_written() const noexcept {
			return digits_written_;
		}
		[[nodiscard, gnu::pure]] std::size_t bytes_written() const noexcept {
			return bytes_written_;
		}
	};


	/** for reading a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned
	integer from a data stream written by a writer with the same type parameters. */
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
		std::size_t digits_read_ {0uz};
		std::size_t bytes_read_ {0uz};

		/** read `num_buf_bytes` bytes from `is`.
		\post `buf_meter_ == buf_t_max`. */
		void read_buf(std::istream& is) {
			buf_meter_ = buf_t_max;
			std::uint_fast8_t bytes_read {0u};
			buf_ = 0u;
			if constexpr (std::endian::native == std::endian::little) {
				is.read(reinterpret_cast<char*>(&buf_), num_buf_bytes);
				bytes_read = static_cast<std::uint_fast8_t>(is.gcount());
			} else if constexpr (std::endian::native == std::endian::big) {
				is.read(reinterpret_cast<char*>(&buf_), num_buf_bytes);
				std::byteswap(buf_);
				bytes_read = static_cast<std::uint_fast8_t>(is.gcount());
			} else {
				for (auto i {0uz}; i < num_buf_bytes; ++i) {
					char c; is.get(c); if (is) {
						buf_ |= buf_t{c} << (i*CHAR_BIT);
						++bytes_read;
					} else { break; }
				}
			}
			OKIIDOKU_CONTRACT_USE(bytes_read <= num_buf_bytes);
			bytes_read_ += bytes_read;
			buf_meter_ >>= CHAR_BIT * (num_buf_bytes - bytes_read);
		}

	public:
		/**
		\pre the data being read was written by a writer with the same type parameters.
		\pre if this is the `n`th call, it is for the `n`th least-significant digit, and
			`radix` is the same value that was passed to write this digit.
		\pre `is` opened with same text/binary mode used when writing this data.
		\pre `radix > 0`.
		*/
		[[nodiscard]] radix_t read(std::istream& is, buf_t radix) {
			OKIIDOKU_CONTRACT_USE(buf_ <= buf_meter_);
			OKIIDOKU_CONTRACT_USE(radix > 0u);
			++digits_read_;
			if (radix < 2u) [[unlikely]] { return 0u; }
			if (buf_meter_ == 0u) [[unlikely]] {
				read_buf(is);
			}
			OKIIDOKU_CONTRACT_USE(buf_meter_ > 0u);
			OKIIDOKU_CONTRACT_USE(buf_ <= buf_meter_);
			radix_t digit {0u};
			if (buf_meter_ < buf_t{radix-1u}) [[unlikely]]/*(but more likely than `buf_meter_ == 0u` (?))*/ { // TODO what if change to <= instead of < ?
				digit += static_cast<radix_t>(buf_);
				radix = static_cast<radix_t>(radix - buf_meter_ + 1u);
				read_buf(is);
			}
			OKIIDOKU_CONTRACT_USE(buf_meter_ >= radix-1u);
			digit += static_cast<radix_t>(buf_ % radix);
			buf_ /= radix;
			buf_meter_ /= radix;
			return digit;
		}

		/** call this once there are no more digits to be read. */
		void finish(std::istream& is) {
			const auto unused_buf_bytes {static_cast<std::uint_fast8_t>(std::bit_width(buf_meter_) / CHAR_BIT)};
			is.seekg(-std::stringstream::off_type{unused_buf_bytes}, std::ios_base::cur);
			bytes_read_ -= unused_buf_bytes;
		}

		[[nodiscard, gnu::pure]] std::size_t digits_read() const noexcept {
			return digits_read_;
		}
		/** \pre this reader has been `finish()`ed. */
		[[nodiscard, gnu::pure]] std::size_t bytes_read() const noexcept {
			return bytes_read_;
		}
	};
}
#endif