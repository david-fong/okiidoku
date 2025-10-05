// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_DETAIL_UTIL
#define HPP_OKIIDOKU_DETAIL_UTIL

#include <cassert> // assert

// TODO.low would it be legal ODR-wise to define the contract stuff as doctest CHECK/REQUIRE if a doctest header include macro is present here?

/**
	\def OKIIDOKU_CONTRACT(expr)
	translates to assertion for debug builds, and assumption for release builds.
	based on https://github.com/microsoft/GSL/blob/main/include/gsl/assert.

	\def OKIIDOKU_CONTRACT2(expr)
	like `OKIIDOKU_CONTRACT`, but use this when the compiler thinks the expression may
	have side-effects. prefer `OKIIDOKU_CONTRACT` when possible.
	\note if the condition expression is costly to evaluate, and the compiler can't
	figure out it has no side-effects, or const pure expressions haven't or won't
	happen anyway, this could backfire for performance, so avoid using this in such
	cases, or benchmark carefully, or check codegen. see `OKIIDOKU_ASSERT`.

	note: C++23 has `std::unreachable` in `<utility>`, but it might be good to keep this header slim */
#ifdef _MSC_VER
	#define OKIIDOKU_DETAIL_CONTRACT_(expr ) __assume(expr) // TODO.wait msvc to support [[assume]]
	#define OKIIDOKU_DETAIL_CONTRACT2_(expr) __assume(expr)
	#define OKIIDOKU_DETAIL_UNREACHABLE_     __assume(false)
#elif defined(__GNUC__) // includes clang (unless -fgnuc=0)
	#define OKIIDOKU_DETAIL_CONTRACT_( expr) [[assume(expr)]]
	#define OKIIDOKU_DETAIL_CONTRACT2_(expr) ((expr) ? static_cast<void>(0) : __builtin_unreachable())
	#define OKIIDOKU_DETAIL_UNREACHABLE_     __builtin_unreachable()
#else
	#pragma warning "okiidoku's contract macros aren't defined for this compiler"
	#define OKIIDOKU_DETAIL_CONTRACT_( expr) static_cast<void>(0)
	#define OKIIDOKU_DETAIL_CONTRACT2_(expr) static_cast<void>(0)
	#define OKIIDOKU_DETAIL_UNREACHABLE_     static_cast<void>(0)
#endif
#ifdef NDEBUG
	#define OKIIDOKU_CONTRACT( expr) OKIIDOKU_DETAIL_CONTRACT_(expr)
	#define OKIIDOKU_CONTRACT2(expr) OKIIDOKU_DETAIL_CONTRACT2_(expr)
	#define OKIIDOKU_UNREACHABLE     OKIIDOKU_DETAIL_UNREACHABLE_
#else
	// NOLINTBEGIN(cert-dcl03-c,misc-static-assert) runtime abort desirable here.
	#define OKIIDOKU_CONTRACT( expr) assert(expr);  OKIIDOKU_DETAIL_CONTRACT_(expr)
	#define OKIIDOKU_CONTRACT2(expr) assert(expr);  OKIIDOKU_DETAIL_CONTRACT2_(expr)
	#define OKIIDOKU_UNREACHABLE     assert(false); OKIIDOKU_DETAIL_UNREACHABLE_
	// NOLINTEND(cert-dcl03-c,misc-static-assert)
#endif
/** \def OKIIDOKU_ASSERT(expr)
alias of standard `assert` */
#define OKIIDOKU_ASSERT(expr) assert(expr); OKIIDOKU_DETAIL_CONTRACT2_(expr)


/// \todo enable only for release builds? or does that thwart ubsan?
#if __cplusplus >= 202403L
	#define OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_INDETERMINATE [[indeterminate]]
#endif
#ifndef OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_INDETERMINATE
	#define OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_INDETERMINATE
#endif
// gnu::uninitialized: don't automatically init variables with automatic storage when -ftrivial-auto-var-init.
#ifdef __clang__
	#define OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED [[clang::uninitialized]]
#elif defined(__GNUC__)
	#define OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED [[gnu::uninitialized]]
#endif
#ifndef OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED
	#define OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED
#endif
/// \note will probably need to pair usage-site with `// NOLINT(*-init)`.
#define OKIIDOKU_DEFER_INIT OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_INDETERMINATE OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED
/// \todo gnu::noinit for global data. linker for ELF will put in .noinit section


#ifndef NDEBUG
	/** \def OKIIDOKU_KEEP_FOR_DEBUG
	- `gnu::retain`: marks for retention during linker section garbage collection
	- `gnu::used`:   I'm actually not sure if this is needed
	- `gnu::externally_visible`: "nullifies the effect of the `-fwhole-program`
		command-line option, so the object remains visible outside the current compilation unit." */
	#define OKIIDOKU_KEEP_FOR_DEBUG [[maybe_unused, gnu::retain, gnu::used, gnu::noinline, gnu::externally_visible]]
#else
	#define OKIIDOKU_KEEP_FOR_DEBUG
#endif


#ifdef __cpp_lib_execution
#define OKIIDOKU_UNSEQ std::execution::unseq,
#else
#define OKIIDOKU_UNSEQ
#endif


#endif
