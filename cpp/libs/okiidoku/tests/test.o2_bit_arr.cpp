// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <catch2/catch_test_macros.hpp>

#include <okiidoku/bit_array.hpp>
#include <okiidoku/bit_array.cpp> // NOLINT(*include*) implementations aren't exported in libokiidoku.
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/detail/util.hpp>

#include <random>
#include <cstdint>
#include <cstddef> // size_t

#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif

namespace okiidoku::test {
	namespace { template<class T> using uidist_t = std::uniform_int_distribution<T>; }

template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_o2_bit_arr_ones() {
	INFO("testing for order " << unsigned{O});
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	REQUIRE(O2BitArr_ones<O>.count() == T::O2);
	for (const auto i : T::O2) { CAPTURE(i);
		REQUIRE(O2BitArr_ones<O>.count_below(i) == i);
		REQUIRE(O2BitArr_ones<O>.nth_set_bit(i) == i);
	}{
		O2BitArr<O> arr {}; CAPTURE(arr);
		for (const auto i : T::O2) { CAPTURE(i);
			REQUIRE(!arr[i]);
			arr.set(i);
			REQUIRE(arr[i]);
		}
	}{
		auto ones {O2BitArr_ones<O>};
		for (const auto i : T::O2) { CAPTURE(i);
			REQUIRE(ones.count_below(i) == 0u);
			REQUIRE(ones.nth_set_bit(0u) == i);
			REQUIRE(ones[*ones.first_set_bit()]);
			ones.unset(*ones.first_set_bit());
		}
	}{
		o2i_t count {0u};
		for (const auto i : O2BitArr_ones<O>.set_bits()) { CAPTURE(i);
			REQUIRE(i == count);
			++count;
		}
		REQUIRE(count == T::O2);
	}
}


template<Order O, IntKind kind_> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_o2_bit_arr_basic() {
	INFO("testing for order " << unsigned{O});
	CAPTURE(kind_);
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	{
		O2BitArr<O,kind_> a;
		REQUIRE(a.count() == 0u);
		REQUIRE(a.first_set_bit() == T::O2);
		for (const auto i : T::O2) { REQUIRE_FALSE(a[i]); }
		for ([[maybe_unused]] const auto b : a.set_bits()) [[unlikely]] { FAIL("no bits set"); }
	}

	for (const auto i_set : T::O2) { CAPTURE(i_set);
		O2BitArr<O,kind_> a;
		REQUIRE(a.count() == 0u);
		for (const auto i : T::O2) { REQUIRE_FALSE(a[i]); }
		for ([[maybe_unused]] const auto i : a.set_bits()) { FAIL(""); }

		a.set(i_set);
		REQUIRE(a[i_set]);
		REQUIRE(a.count() == 1u);
		for (const auto i : T::O2) { REQUIRE(a[i] == (i == i_set)); }
		for (const auto i : a.set_bits()) { REQUIRE(i == i_set); }
		REQUIRE(a.first_set_bit() == i_set);
		REQUIRE(a.count_below(i_set) == 0u);
		REQUIRE(a.nth_set_bit(0u) == i_set);

		a.unset(i_set);
		REQUIRE_FALSE(a[i_set]);
		REQUIRE(a.count() == 0u);
		for (const auto i : T::O2) { REQUIRE_FALSE(a[i]); }
		for ([[maybe_unused]] const auto i : a.set_bits()) { FAIL(""); }

		a.flip(i_set);
		REQUIRE(a[i_set]);
		REQUIRE(a.count() == 1u);
		for (const auto i : T::O2) { REQUIRE(a[i] == (i == i_set)); }
		for (const auto i : a.set_bits()) { REQUIRE(i == i_set); }

		auto a2 {a};
		REQUIRE(a == a2);
		for (const auto i : T::O2) { a2.flip(i); }
		REQUIRE(a == ~a2);
		REQUIRE(~a == a2);
	}
}


template<Order O, IntKind kind_> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_o2_bit_arr_rand(const std::uint_fast32_t rng_seed) {
	INFO("testing for order " << unsigned{O});
	CAPTURE(kind_);
	CAPTURE(rng_seed);
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
	std::minstd_rand rng {rng_seed};

	const auto num_set_bits {uidist_t<std::size_t>{1uz, T::O2}(rng)};
	O2BitArr<O,kind_> a;
	for (const auto i_ : o2i_t{num_set_bits}) { CAPTURE(i_);
		// find a bit to set that isn't set yet (and check some consistency with `operator~`)
		const auto a_inv {~a};
		REQUIRE(a_inv.count() == T::O2-a.count());
		REQUIRE(a_inv == a_inv);
		REQUIRE(a_inv == ~a);
		REQUIRE(~a == a_inv);
		{
			auto to_zero {a};
			to_zero.retain_only(a_inv);
			REQUIRE(to_zero.count() == 0u);
		}{
			auto to_zero {a_inv};
			to_zero.retain_only(a);
			REQUIRE(to_zero.count() == 0u);
		}{
			auto same {a};
			same.remove(a_inv);
			REQUIRE(same == a);
		}{
			auto same {a_inv};
			same.remove(a);
			REQUIRE(same == a_inv);
		}{
			const auto zeroes {a & a_inv};
			REQUIRE(zeroes.count() == 0u);
		}{
			const auto zeroes {a_inv & a};
			REQUIRE(zeroes.count() == 0u);
		}{
			const auto ones {a | a_inv};
			REQUIRE(ones.count() == T::O2);
		}{
			const auto ones {a_inv | a};
			REQUIRE(ones.count() == T::O2);
		}
		const auto i_set {a_inv.nth_set_bit(uidist_t<std::size_t>{0uz, a_inv.count()-1uz}(rng))};
		REQUIRE_FALSE(a[i_set]);
		a.set(i_set);
		REQUIRE(a[i_set]);
		REQUIRE(a.count() == i_+int1);
		REQUIRE(a == a);
	}
	{
		REQUIRE(a.count() == num_set_bits);
		for (const auto i : a.set_bits()) { CAPTURE(i);
			REQUIRE(a.nth_set_bit(a.count_below(i)) == i);
		}
		for (const auto i : o2i_t{a.count()}) { CAPTURE(i);
			REQUIRE(a.count_below(a.nth_set_bit(i)) == i);
		}
	}
}}

TEST_CASE("okiidoku.o2_bit_arr") {
	static constexpr std::uintmax_t num_rounds {1024u};
	// std::mt19937 rng {0u};
	std::mt19937 rng {std::random_device{}()};
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_o2_bit_arr_ones<(O_)>(); \
	for (std::uintmax_t round {0u}; round < num_rounds; ++round) { CAPTURE(round); \
		const auto seed {rng()}; \
		okiidoku::test::test_o2_bit_arr_basic<(O_),okiidoku::IntKind::fast >(); \
		okiidoku::test::test_o2_bit_arr_basic<(O_),okiidoku::IntKind::small>(); \
		okiidoku::test::test_o2_bit_arr_rand<(O_),okiidoku::IntKind::fast >(seed); \
		okiidoku::test::test_o2_bit_arr_rand<(O_),okiidoku::IntKind::small>(seed); \
	}
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}