// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_DETAIL_MIXED_RADIX_INT_SERDES
#define HPP_OKIIDOKU_DETAIL_MIXED_RADIX_INT_SERDES
#include <okiidoku/detail/util.hpp>

#include <iosfwd>
#include <concepts>
#include <utility>
#include <limits>   // numeric_limits, ...
#include <bit>      // byteswap, bit_width
#include <cstdint>
#include <climits>  // CHAR_BIT

#define TO static_cast

/*
\todo.low I pondered moving some stuff into a base class, like item_count_, byte_count, buf_radixx_
and some of the logic like overflow anticipation calculation, and calculation of radix_0, but then I
think I wouldn't have control over member layout w.r.t. the inheritence boundary... am I just being too
picky?

\internal the binary stream requirement is because the C++ standard has tight requirements for written
data to get reproducible read. see https://en.cppreference.com/w/cpp/io/c/FILE.html#Binary_and_text_modes.
*/

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
	class MixedRadixUintWriter final { // NOLINT(*-member-init) `queue_`
	public:
		using radix_t = RadixType_;
		using buf_t = BufType_;
		struct [[gnu::designated_init]] Item final {
			radix_t radix;
			radix_t digit;
		};
	private:
		static constexpr radix_t radix_t_max {std::numeric_limits<radix_t>::max()};
		static constexpr buf_t   buf_t_max   {std::numeric_limits<buf_t  >::max()};
			static_assert(radix_t_max-1u <= buf_t_max);

		std::size_t item_count_ {0uz};
		std::size_t byte_count_ {0uz};
		buf_t buf_radixx_   {0u};    //< write buffer if all digits were maxed-out. upwards-growing. "x" := "exclusive"
		bool  did_overflow_ {false}; // TODO is it possible to express this by whether `queue[queue_end_].radix == 0u`?
		std::uint_fast8_t queue_end_ {0u};
		std::array<Item, (sizeof(buf_t)*CHAR_BIT)+1uz> queue_; // lower indices are for earlier-passed items

	public:
		/** accept a digit for writing. call this once for each digit of the integer.
		digit visitation order is up to the caller to determine and keep consistent between read/write.
		\return `true` if a `flush` isn't needed yet; `false` if a `flush` is now needed.
		\pre if this is the `n`th call, it is for the `n`th digit.
		\pre `digit < radix`.
		\pre `radix > 0`.
		\pre the previous call didn't indicate a need for a `flush`. */
		[[nodiscard]] bool accept(Item in) noexcept {
			OKIIDOKU_CONTRACT(!did_overflow_);              // caller `flush`ed as needed.
			OKIIDOKU_CONTRACT(buf_radixx_ <= buf_t_max/2u); // caller `flush`ed as needed.
			OKIIDOKU_CONTRACT2(queue_end_ < queue_.size());
			OKIIDOKU_CONTRACT(in.radix <= radix_t_max);
			OKIIDOKU_CONTRACT(in.radix > 0u);
			OKIIDOKU_CONTRACT(in.digit < in.radix);
			++item_count_;
			if (in.radix < 2u) [[unlikely]] { OKIIDOKU_ASSERT(in.digit == 0u); return true; }
			if ((buf_radixx_ > buf_t_max/in.radix) || (in.radix-1u > TO<buf_t>(buf_t_max-(buf_radixx_*in.radix)))) [[unlikely]] {
				// handle overflow of `flush` buffer...
				OKIIDOKU_CONTRACT(buf_radixx_ != 0u); // (see template params constraint)
				// try to use remaining space in buf:
				const auto radix_0 {TO<radix_t>(buf_t_max / (buf_radixx_+1u))}; // won't overflow (buf_radixx_ <= buf_t_max/2u)
					// TODO ^investigate would this always be a power of 2?
				OKIIDOKU_CONTRACT(radix_0 <= radix_t_max);
				OKIIDOKU_CONTRACT(radix_0 < in.radix);
				if (radix_0 >= 2u) [[likely]] {
					buf_radixx_ = TO<buf_t>((buf_radixx_*radix_0)+(radix_0-1u));
					OKIIDOKU_CONTRACT2(queue_end_ < queue_.size()-1uz);
					auto& qi {queue_[queue_end_++]};
					qi.radix = radix_0;
					qi.digit = std::min(in.digit, TO<radix_t>(radix_0 - radix_t{1u}));
					in.radix = TO<radix_t>((in.radix-radix_0)+1u);
					in.digit -= qi.digit;
				}
				OKIIDOKU_CONTRACT(in.radix >= 2u);
				OKIIDOKU_CONTRACT(in.digit < in.radix);
				// save overflow and return:
				// buf_radixx_ = // <- don't! see `flush` uses and resets it.
				OKIIDOKU_CONTRACT2(queue_end_ < queue_.size());
				queue_[queue_end_] = in;
				did_overflow_ = true;

				return false;
			}
			buf_radixx_ = TO<buf_t>((buf_radixx_*in.radix)+(in.radix-1u));
			queue_[queue_end_++] = in;

			static constexpr buf_t buf_t_top_bit_mask {buf_t{1u} << (sizeof(buf_t)*CHAR_BIT-1u)};
			OKIIDOKU_CONTRACT(
				(buf_radixx_ <= buf_t_max/2u)
				== ((buf_radixx_ & buf_t_top_bit_mask) == 0u)
			);
			return (buf_radixx_ & buf_t_top_bit_mask) == 0u;
			// ^whether a minimal, non-trivial `accept` could happen without overflow.
			//  i.e. `buf_radixx_ <= buf_t_max/2u`
		}

		/**
		call this exactly once when the previous call to `accept` indicated a need to flush.
		separately, call this exactly once after `accept()`ing the final digit.
		\pre `os` is a binary stream- not a text stream.
		\pre the previous call to `accept` indicated a need to flush, or there is no more data. */
		void flush(std::ostream& os) {
			OKIIDOKU_CONTRACT2(os.good());
			OKIIDOKU_CONTRACT2(queue_end_ < queue_.size());
			if (queue_end_ == 0u) [[unlikely]] { return; }
			OKIIDOKU_CONTRACT(buf_radixx_ > 0u);

			// prepare the real write buffer:
			buf_t buf {0u};
			for (auto i {0uz}; i < queue_end_; ++i) {
				OKIIDOKU_CONTRACT(i+1uz <= queue_end_);
				const auto& qi {queue_[queue_end_-1uz-i]};
				OKIIDOKU_CONTRACT(qi.radix > 0u);
				OKIIDOKU_CONTRACT(qi.digit < qi.radix);
				buf *= qi.radix;
				buf += qi.digit;
			}
			OKIIDOKU_CONTRACT(buf <= buf_radixx_);

			// write `buf` to the stream:
			const auto num_bytes {TO<std::uint_fast8_t>((std::bit_width(buf_radixx_)+(CHAR_BIT-1)) / CHAR_BIT)};
			OKIIDOKU_CONTRACT(num_bytes <= sizeof(buf_t));
			OKIIDOKU_CONTRACT(num_bytes > 0u);
			byte_count_ += num_bytes;
			if constexpr (std::endian::native == std::endian::little) {
				static_assert(alignof(buf_t) >= alignof(char));
				os.write(reinterpret_cast<const char*>(&buf), num_bytes); // NOLINT(*-cast)
			}
			// if (false) {}
			else { for (auto i {0uz}; i < num_bytes; ++i) {
				os.put(TO<char>(buf));
				buf >>= CHAR_BIT;
			}}

			if (did_overflow_) [[likely]] {
				did_overflow_ = false;
				queue_.front() = queue_[queue_end_];
				queue_end_ = 1u;
				const auto& qi {queue_.front()};
				OKIIDOKU_CONTRACT(qi.radix > 0u);
				OKIIDOKU_CONTRACT(qi.digit < qi.radix);
				buf_radixx_ = TO<buf_t>(qi.radix-1u);
			} else {
				queue_end_ = 0u;
				buf_radixx_ = 0u;
			}
		}

		[[nodiscard, gnu::pure]] std::size_t item_count() const noexcept {
			return item_count_;
		}
		[[nodiscard, gnu::pure]] std::size_t byte_count() const noexcept {
			return byte_count_;
		}
	};


	// TODO.high fix bug. for the last read_buf, reader needs to know how many bytes need to be read and not overread.
	// it worked when I tested only one logical MRUI to a stream because reading would hit EOF. there are various ways
	// I could address this (ex. reserving a bit per byte to signal whether the byte is the last one, or making a zero-
	// byte instead be a zero byte plus a following byte with such a signal bit, or by making the caller give the reader
	// the length of data to read up-front).
	/** for reading a [mixed-radix](https://wikipedia.org/wiki/Mixed_radix) unsigned
	integer from a data stream written by a writer with the same type parameters. */
	template<Radix RadixType_, std::unsigned_integral BufType_ = std::uintmax_t>
		requires(std::numeric_limits<RadixType_>::max()-1u <= std::numeric_limits<BufType_>::max())
		// ^this greatly simplifies implementation. if this restriction is changed, overflow handling and int casts will need to change.
	class MixedRadixUintReader final {
	public:
		using radix_t = RadixType_;
		using buf_t = BufType_;
	private:
		static constexpr radix_t radix_t_max {std::numeric_limits<radix_t>::max()};
		static constexpr buf_t   buf_t_max   {std::numeric_limits<buf_t  >::max()};
			static_assert(radix_t_max-1u <= buf_t_max);

		std::size_t item_count_ {0uz};
		std::size_t byte_count_ {0uz};
		buf_t buf_radixx_ {buf_t_max}; //< write buffer if all digits were maxed-out. upwards-growing. "x" := "exclusive".
		buf_t buf_ {0u}; //< buffer of data from input stream. downwards-depleting.

		/** read `sizeof(buf_t)` bytes from `is`.
		\pre `is.good()` and `is.exceptions() == std::ios::goodbit`.
		\post iff no data could be read from `is`, `is` has `eofbit` and `failbit`.
		\internal conditions to call this correspond to conditions where writer `accept`
			indicates need for caller to `flush`. */
		void read_buf(std::istream& is) noexcept {
			OKIIDOKU_CONTRACT2(is.good());
			OKIIDOKU_CONTRACT2(is.exceptions() == std::ios::goodbit);
			if (is.rdbuf() != nullptr && is.rdbuf()->in_avail() < static_cast<long>(sizeof(buf_t))) [[unlikely]] {
				is.sync();
				/* I'm not sure if this is necessary, but hoping that it prevents issues
				which could be caused by the `seekg()` in `finish` if overread data crosses
				a rdbuf boundary, and stream sequence can't remember past-buffer data,
				if that's a thing, like with a network stream sequence. or maybe instead
				of seekg, the putback functions could address this? */
			}
			buf_radixx_ = 0u;
			std::size_t byte_count {0uz};
			buf_ = 0u;
			// note: `is.read` and `is.get` set `eofbit|failbit` if can't read expected count.
			if constexpr (std::endian::native == std::endian::little) {
				static_assert(alignof(buf_t) >= alignof(char));
				is.read(reinterpret_cast<char*>(&buf_), sizeof(buf_t)); // NOLINT(*-cast)
				byte_count = TO<std::size_t>(is.gcount());
			}
			else if constexpr (std::endian::native == std::endian::big) {
				static_assert(alignof(buf_t) >= alignof(char));
				is.read(reinterpret_cast<char*>(&buf_), sizeof(buf_t)); // NOLINT(*-cast)
				std::byteswap(buf_);
				byte_count = TO<std::size_t>(is.gcount());
			}
			// if (false) {}
			else { for (auto i {0uz}; i < sizeof(buf_t); ++i) {
				if (char c OKIIDOKU_DEFER_INIT; is.get(c), is) [[likely]] { // NOLINT(*init*)
					buf_ |= TO<buf_t>( TO<buf_t>(TO<unsigned char>(c)) << (i*CHAR_BIT) ); // good grief- the casting needed here =_=
					++byte_count;
				} else { break; }
			}}
			if (byte_count > 0u && !is) [[unlikely]] { is.clear(); }
			/* ^unless we failed to read _anything_, swallow `eofbit` and `failbit`, if set.
			it's the caller's job to know expected end. we don't know here if overreading. */
			OKIIDOKU_CONTRACT(byte_count <= sizeof(buf_t));
			byte_count_ += byte_count; // TODO.high what if bytes read is 0? return error val here? make `read()` return `std::expected`?
		}

	public:
		/**
		\returns the previously written `n`th digit, or `radix` upon unexpected EOF.
		digit visitation order is up to the caller to determine and keep consistent between read/write.
		\pre the data being read was written by a writer with the same type parameters.
		\pre if this is the `n`th call, it is for the `n`th digit, and `radix` is the
			same value that was passed to write this digit.
		\pre `is` is a binary stream- not a text stream.
		\pre `is.good()` and `is.exceptions() == std::ios::goodbit`.
		\pre `radix > 0`.
		\post if hit unexpected EOF, `is` has `eofbit|failbit` set.
		\post if didn't hit unexpected EOF, the return value is less than `radix`. */
		[[nodiscard]] radix_t read(std::istream& is, radix_t radix) noexcept {
			OKIIDOKU_CONTRACT(buf_t_max-buf_radixx_ >= buf_);
			OKIIDOKU_CONTRACT2(is.good());
			OKIIDOKU_CONTRACT2(is.exceptions() == std::ios::goodbit);
			OKIIDOKU_CONTRACT(radix <= radix_t_max);
			OKIIDOKU_CONTRACT(radix > 0u);
			[[maybe_unused]] const auto radix_orig {radix};
			++item_count_;
			if (radix < 2u) [[unlikely]] { return 0u; }
			static constexpr buf_t buf_t_top_bit_mask {buf_t{1u} << (sizeof(buf_t)*CHAR_BIT-1u)};
			if (buf_radixx_ & buf_t_top_bit_mask) [[unlikely]] { // is top bit set (buf exhausted)
				read_buf(is);
				if (!is) [[unlikely]] { return radix; }
			}
			OKIIDOKU_CONTRACT(buf_radixx_ <= buf_t_max/2u);
			OKIIDOKU_CONTRACT(buf_t_max-buf_radixx_ >= buf_);
			OKIIDOKU_CONTRACT(buf_t_max-buf_radixx_ >= TO<buf_t>(radix-1u));
			radix_t digit {0u};
			if ((buf_radixx_ > buf_t_max/radix) || (radix-1u > TO<buf_t>(buf_t_max-(buf_radixx_*radix)))) [[unlikely]] {
				OKIIDOKU_CONTRACT(buf_radixx_ != 0u); // (see template params constraint)
				const auto radix_0 {TO<radix_t>(buf_t_max / (buf_radixx_+1u))}; // should divisor be +1? must follow writer
				OKIIDOKU_CONTRACT(radix_0 <= radix_t_max);
				OKIIDOKU_CONTRACT(radix_0 <  radix);
				// buf_radixx_ = ; // no point. `read_buf` will overwrite it and doesn't use it.
				radix  = TO<radix_t>((radix-radix_0)+1u);
				digit += TO<radix_t>(buf_);
				OKIIDOKU_CONTRACT(digit < radix_orig);
				read_buf(is);
				if (!is) [[unlikely]] { return radix; }
			}
			OKIIDOKU_CONTRACT(radix >= 2u);
			OKIIDOKU_CONTRACT(buf_t_max-buf_radixx_ >= buf_);
			OKIIDOKU_CONTRACT(buf_t_max-buf_radixx_ >= TO<buf_t>(radix-1u));
			buf_radixx_ = TO<buf_t>((buf_radixx_*radix)+(radix-1u));
			digit += TO<radix_t>(buf_ % radix);
			buf_  /= radix;
			OKIIDOKU_CONTRACT(digit < radix_orig);
			return digit;
		}

		/** call this once when there are no more digits to be read.
		do not call again afterwards.
		\pre all expected data has been fully, successfully read.
		\pre `is.good()` and `is.exceptions() == std::ios::goodbit`. */
		void finish(std::istream& is) noexcept {
			OKIIDOKU_CONTRACT2(is.good());
			OKIIDOKU_CONTRACT2(is.exceptions() == std::ios::goodbit);
			using u8_t = std::uint_fast8_t;

			const auto bytes_last_read {[&]{
				auto v {TO<u8_t>(byte_count_ % sizeof(buf_t))};
				if (v == 0u) [[likely]] { v = sizeof(buf_t); }
				return v;
			}()};
			OKIIDOKU_CONTRACT(bytes_last_read > 0u);
			OKIIDOKU_CONTRACT(bytes_last_read <= sizeof(buf_t));

			const auto buf_bytes_used {[&]{
				OKIIDOKU_CONTRACT(buf_radixx_ > 0u); // `read_buf` always followed by some buffer consumption.
				// const auto buf_bytes_unused {TO<u8_t>(std::countl_zero(buf_radixx_) / CHAR_BIT)};
				// OKIIDOKU_CONTRACT(buf_bytes_unused < sizeof(buf_t));
				// return TO<u8_t>(sizeof(buf_t) - buf_bytes_unused);
				return TO<u8_t>((std::bit_width(buf_radixx_)+(CHAR_BIT-1)) / CHAR_BIT);
			}()};
			OKIIDOKU_CONTRACT(buf_bytes_used > 0u);
			OKIIDOKU_CONTRACT(buf_bytes_used <= bytes_last_read);

			const auto unused_read_bytes {TO<u8_t>(bytes_last_read - buf_bytes_used)};
			OKIIDOKU_CONTRACT(unused_read_bytes < sizeof(buf_t));

			is.seekg(-std::istream::off_type{unused_read_bytes}, std::ios::cur); // (clears `eofbit`)
			OKIIDOKU_CONTRACT(byte_count_ >= unused_read_bytes);
			byte_count_ -= unused_read_bytes;
		}

		[[nodiscard, gnu::pure]] std::size_t item_count() const noexcept {
			return item_count_;
		}
		/** \pre this reader has been `finish()`ed. */
		[[nodiscard, gnu::pure]] std::size_t byte_count() const noexcept {
			return byte_count_;
		}
	};
}
#undef TO
#endif