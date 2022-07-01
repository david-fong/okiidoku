#ifndef HPP_OKIIDOKU__DETAIL__CONTRACT
#define HPP_OKIIDOKU__DETAIL__CONTRACT

#include <cassert>

// OKIIDOKU_CONTRACT_USE
// translates to assertion for debug builds.
// translates to assumption for release builds.
// if the condition expression is complex to evaluate, this may possibly backfire
// for performance, so avoid using this in such cases, or benchmark carefully.

#ifdef NDEBUG
// based on https://github.com/microsoft/GSL/blob/main/include/gsl/assert
	#ifdef _MSC_VER
	#define OKIIDOKU_CONTRACT_USE(cond) __assume(cond)

	#elif defined(__GNUC__)
	#define OKIIDOKU_CONTRACT_USE(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())

	#else
	#define OKIIDOKU_CONTRACT_USE(cond) static_cast<void>((cond) ? 0 : 0)
	#endif

#else
	#define OKIIDOKU_CONTRACT_USE(expr) assert(expr)
#endif

#endif


// OKIIDOKU_CONTRACT_ASSERT
// alias of standard `assert`
#define OKIIDOKU_CONTRACT_ASSERT(expr) assert(expr)