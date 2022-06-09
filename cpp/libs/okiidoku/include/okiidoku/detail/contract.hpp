#ifndef HPP_OKIIDOKU__DETAIL__CONTRACT
#define HPP_OKIIDOKU__DETAIL__CONTRACT

#ifdef NDEBUG
// based on https://github.com/microsoft/GSL/blob/main/include/gsl/assert
	#ifdef _MSC_VER
	#define OKIIDOKU_CONTRACT_TRIVIAL_EVAL(cond) __assume(cond)

	#elif defined(__GNUC__)
	#define OKIIDOKU_CONTRACT_TRIVIAL_EVAL(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())

	#else
	#define OKIIDOKU_CONTRACT_TRIVIAL_EVAL(cond) static_cast<void>((cond) ? 0 : 0)
	#endif

#else
	#include <cassert>
	#define OKIIDOKU_CONTRACT_TRIVIAL_EVAL(expr) assert(expr)
#endif

#endif