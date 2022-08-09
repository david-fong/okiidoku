// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__DETAIL__CONTRACT
#define HPP_OKIIDOKU__DETAIL__CONTRACT

#include <cassert>
#include <cstdlib>

// OKIIDOKU_CONTRACT_USE
// translates to assertion for debug builds.
// translates to assumption for release builds.
// if the condition expression is complex to evaluate, this may possibly backfire
// for performance, so avoid using this in such cases, or benchmark carefully.

#ifdef NDEBUG
// based on https://github.com/microsoft/GSL/blob/main/include/gsl/assert
	#ifdef _MSC_VER
	#define OKIIDOKU_CONTRACT_USE(cond) __assume(cond)
	#define OKIIDOKU_UNREACHABLE __assume(false)
	#elif defined(__GNUC__)
	#define OKIIDOKU_CONTRACT_USE(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
	#define OKIIDOKU_UNREACHABLE __builtin_unreachable()
	#else
	#define OKIIDOKU_CONTRACT_USE(cond) static_cast<void>((cond) ? 0 : 0)
	#define OKIIDOKU_UNREACHABLE
	#endif
#else
	#define OKIIDOKU_CONTRACT_USE(expr) assert(expr) // NOLINT(cert-dcl03-c,misc-static-assert) runtime abort desirable here.
	#define OKIIDOKU_UNREACHABLE std::abort()
#endif


// OKIIDOKU_CONTRACT_ASSERT
// alias of standard `assert`
#define OKIIDOKU_CONTRACT_ASSERT(expr) assert(expr)

#endif