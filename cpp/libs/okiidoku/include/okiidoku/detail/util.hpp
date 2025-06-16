// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__UTIL
#define HPP_OKIIDOKU__DETAIL__UTIL

#include <cassert> // assert

/*
OKIIDOKU_CONTRACT_USE
translates to assertion for debug builds.
translates to assumption for release builds.
if the condition expression is complex to evaluate, this may possibly backfire
for performance, so avoid using this in such cases, or benchmark carefully.
*/

// Note: C++23 has std::unreachable in <utility>, but it might be good to keep this header slim
#ifdef NDEBUG
// based on https://github.com/microsoft/GSL/blob/main/include/gsl/assert
	#ifdef _MSC_VER
	#define OKIIDOKU_CONTRACT_USE(cond) __assume(cond)
	#define OKIIDOKU_UNREACHABLE __assume(false)
	#elif defined(__GNUC__) // includes clang (unless -fgnuc=0)
	#define OKIIDOKU_CONTRACT_USE(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
	#define OKIIDOKU_UNREACHABLE __builtin_unreachable()
	#else
	#define OKIIDOKU_CONTRACT_USE(cond) static_cast<void>((cond) ? 0 : 0)
	#define OKIIDOKU_UNREACHABLE
	#endif
#else
	#define OKIIDOKU_CONTRACT_USE(expr) assert(expr) // NOLINT(cert-dcl03-c,misc-static-assert) runtime abort desirable here.
	#include <cstdlib> // abort
	#define OKIIDOKU_UNREACHABLE std::abort()
#endif


// OKIIDOKU_CONTRACT_ASSERT
// alias of standard `assert`
#define OKIIDOKU_CONTRACT_ASSERT(expr) assert(expr)


// TODO enable only for release builds
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
#define OKIIDOKU_NO_PRE_INIT_AUTOVAR OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_INDETERMINATE OKIIDOKU_DETAIL_NO_PRE_INIT_AUTOVAR_GCC_UNINITIALIZED
// TODO: gnu::noinit for global data. linker for ELF will put in .noinit section


#ifdef NDEBUG
	// gnu::retain: marks for retention during linker section garbage collection
	// gnu::used:   I'm actually not sure if this is needed
	#define OKIIDOKU_KEEP_FOR_DEBUG [[maybe_unused, gnu::retain, gnu::used]]
#else
	#define OKIIDOKU_KEEP_FOR_DEBUG
#endif


#endif
