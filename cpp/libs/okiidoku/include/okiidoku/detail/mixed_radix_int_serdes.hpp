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
	concept Radix = std::numeric_limits<T>::is_specialized
		&&  std::numeric_limits<T>::is_integer
		&& !std::numeric_limits<T>::is_signed
		&&  std::numeric_limits<T>::round_style == std::round_toward_zero
		&&  std::numeric_limits<T>::max() > 1u
		;

	/** for writing a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned integer to a stream.
	\note this is not a generalized interface for mixed-radix integers. the reader
		interface is greedy to allow later radices to be determined based on preceding
		digit values- a property the Grid serdes facilities rely upon. otherwise, the
		implementation here could try to optimize layout/grouping for better packing. */
	template<Radix RadixType_, std::unsigned_integral BufType_ = std::uintmax_t>
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

		std::array<Item, (sizeof(buf_t)*CHAR_BIT)+1uz> queue_; // lower indices are for earlier-passed items
		std::uint_fast8_t queue_end_ {0u};
		bool did_overflow_ {false};
		buf_t buf_meter_ {0u}; /** \internal note: the sole runtime variable this depends on at any moment is the sequence of input radices thus far. */
		std::size_t digits_written_ {0uz};
		std::size_t bytes_written_ {0uz};
		// buf = q[0].d + q[1].d*q[0].r + q[2].d*q[1].r*q[0].r + q[3].d*q[2].r*q[1].r*q[0].r
		// buf += q[3].d; buf *= q[2].r; buf += q[2].d; buf *= q[1].r; buf += q[1].d; buf *= q[0].r; buf += q[0].d;

	public:
		/** accept a digit for writing. call this once for each digit of the integer.
		digit visitation order is up to the caller to determine and keep consistent between read/write.
		\return `true` if a `flush` isn't needed yet; `false` if a `flush` is now needed.
		\pre if this is the `n`th call, it is for the `n`th digit.
		\pre `digit < radix`.
		\pre `radix > 0`.
		\pre the previous call didn't indicate a need for a `flush`. */
		[[nodiscard]] bool accept(Item in) noexcept {
			OKIIDOKU_CONTRACT_USE(buf_meter_ < buf_t_max); // callee should have `flush`ed otherwise (see non-overflow return path)
			OKIIDOKU_CONTRACT_USE(!did_overflow_);         // callee should have `flush`ed otherwise (see overflow return path)
			OKIIDOKU_CONTRACT_USE(queue_end_+1uz <= queue_.size());
			OKIIDOKU_CONTRACT_USE(in.radix <= radix_t_max);
			OKIIDOKU_CONTRACT_USE(in.radix > 0u);
			OKIIDOKU_CONTRACT_USE(in.digit < in.radix);
			++digits_written_;
			if (in.radix < 2u) [[unlikely]] { OKIIDOKU_CONTRACT_ASSERT(in.digit == 0u); return true; }
			if ((buf_meter_ > buf_t_max/in.radix) || (in.radix-1u > static_cast<buf_t>(buf_t_max-(buf_meter_*in.radix)))) [[unlikely]] {
				// handle overflow of `flush` buffer...
				OKIIDOKU_CONTRACT_USE(buf_meter_ != 0u);
				// try to use remaining space in buf:
				const auto radix_0 {static_cast<radix_t>(buf_t_max / (buf_meter_+1u))};
				OKIIDOKU_CONTRACT_USE(radix_0 <= radix_t_max);
				OKIIDOKU_CONTRACT_USE(radix_0 < in.radix);
				if (radix_0 >= 2u) [[likely]] {
					buf_meter_ = static_cast<buf_t>((buf_meter_*radix_0)+(radix_0-1u));
					OKIIDOKU_CONTRACT_USE(queue_end_ < queue_.size()-1uz);
					auto& qi {queue_[queue_end_++]};
					qi.radix = radix_0;
					qi.digit = std::min(in.digit, static_cast<radix_t>(radix_0 - radix_t{2u}));
					in.radix = static_cast<radix_t>((in.radix-radix_0) + 2u);
					in.digit -= qi.digit;
				}
				OKIIDOKU_CONTRACT_USE(in.radix >= 2u);
				OKIIDOKU_CONTRACT_USE(in.digit < in.radix);
				// save overflow and return:
				// buf_meter_ = // <- don't! see `flush` uses and resets it.
				OKIIDOKU_CONTRACT_USE(queue_end_ < queue_.size());
				queue_[queue_end_] = in;
				did_overflow_ = true;
				return false;
			}
			buf_meter_ = static_cast<buf_t>((buf_meter_*in.radix)+(in.radix-1u));
			queue_[queue_end_++] = in;
			return buf_meter_ < buf_t_max; // TODO.high or more efficiently, buf_meter_ <= buf_t_max/2u (equivalent to testing whether most-significant bit is zero)
		}

		/**
		call this exactly once when the previous call to `accept` indicated a need to flush.
		separately, call this exactly once after `accept()`ing the final digit.
		\pre the previous call to `accept` indicated a need to flush, or there is no more data. */
		void flush(std::ostream& os) {
			if (queue_end_ == 0u) [[unlikely]] { return; }

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
			OKIIDOKU_CONTRACT_USE(num_bytes <= sizeof(buf_t));
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
				buf_meter_ = static_cast<buf_t>(qi.radix-1u);
			} else {
				buf_meter_ = 0u;
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
	template<Radix RadixType_, std::unsigned_integral BufType_ = std::uintmax_t>
	class MixedRadixUintReader {
	public:
		using radix_t = RadixType_;
		using buf_t = BufType_;
	private:
		static constexpr buf_t buf_t_max {};
		buf_t buf_ {0u};
		buf_t buf_meter_ {0u}; // TODO.high instead of going downwards, why not mirror writer and go upwards? might be more accurate. I think I'm having accuracy/correctness issues due to this seemingly arbitrary difference of direction
		std::size_t digits_read_ {0uz};
		std::size_t bytes_read_ {0uz};

		/** read `sizeof(buf_t)` bytes from `is`. */
		void read_buf(std::istream& is) {
			buf_meter_ = std::numeric_limits<buf_t>::max();
			std::size_t bytes_read {0uz};
			buf_ = 0u;
			if constexpr (std::endian::native == std::endian::little) {
				is.read(reinterpret_cast<char*>(&buf_), sizeof(buf_t));
				bytes_read = static_cast<std::size_t>(is.gcount());
			} else if constexpr (std::endian::native == std::endian::big) {
				is.read(reinterpret_cast<char*>(&buf_), sizeof(buf_t));
				std::byteswap(buf_);
				bytes_read = static_cast<std::size_t>(is.gcount());
			} else {
				for (auto i {0uz}; i < sizeof(buf_t); ++i) {
					char c; is.get(c); if (is) [[likely]] {
						buf_ |= (buf_t{c} << (i*CHAR_BIT));
						++bytes_read;
					} else { break; }
				}
			}
			OKIIDOKU_CONTRACT_USE(bytes_read <= sizeof(buf_t));
			bytes_read_ += bytes_read; // TODO.high what if bytes read is 0? return error val here? make `read()` return `std::expected`?
			buf_meter_ >>= CHAR_BIT * (sizeof(buf_t) - bytes_read);
		}

	public:
		/**
		\returns the previously written `n`th digit.
		digit visitation order is up to the caller to determine and keep consistent between read/write.
		\pre the data being read was written by a writer with the same type parameters.
		\pre if this is the `n`th call, it is for the `n`th digit, and `radix` is the
			same value that was passed to write this digit.
		\pre `is` opened with same text/binary mode used when writing this data.
		\pre `radix > 0`. */
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
			if (buf_meter_ < radix-1u || (buf_meter_ == radix-1u && buf_ < buf_meter_)) [[unlikely]]/*(but more likely than `buf_meter_ == 0u` (?))*/ {
				digit += static_cast<radix_t>(buf_);
				radix  = static_cast<radix_t>(radix - buf_meter_ + 2u);
				read_buf(is);
			}
			OKIIDOKU_CONTRACT_USE(radix >= 2u);
			OKIIDOKU_CONTRACT_USE(buf_meter_ >= radix-1u);
			digit += static_cast<radix_t>(buf_ % radix);
			buf_ /= radix;
			buf_meter_ /= radix;
			return digit;
		}

		/** call this once there are no more digits to be read.
		do not call again afterwards. */
		void finish(std::istream& is) {
			const auto unused_buf_bytes {static_cast<std::uint_fast8_t>(
				std::bit_width(static_cast<buf_t>(buf_meter_>>1u))
				/ CHAR_BIT
			)};
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