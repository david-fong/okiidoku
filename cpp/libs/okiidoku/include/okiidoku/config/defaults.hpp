#ifndef HPP_OKIIDOKU__CONFIG__DEFAULTS
#define HPP_OKIIDOKU__CONFIG__DEFAULTS
// https://vector-of-bool.github.io/2020/10/04/lib-configuration.html
#if __has_include(<okiidoku_compiled_orders.tweaks.h>)
	#include <compiled_orders.tweaks.h>
#endif

// apologies for the macros. they're used to selectively instantiate templates,
// which allow allocating frequently-used data on the stack instead of the heap.

// TODO.high see if these can be changed to use the constexpr variable flavour of tweak config headers.
#ifndef OKIIDOKU_COMPILE_O2
#define OKIIDOKU_COMPILE_O2 false
#endif

#ifndef OKIIDOKU_COMPILE_O3
#define OKIIDOKU_COMPILE_O3 true
#endif

#ifndef OKIIDOKU_COMPILE_O4
#define OKIIDOKU_COMPILE_O4 true
#endif

#ifndef OKIIDOKU_COMPILE_O5
#define OKIIDOKU_COMPILE_O5 true
#endif

#ifndef OKIIDOKU_COMPILE_O6
#define OKIIDOKU_COMPILE_O6 false
#endif

#ifndef OKIIDOKU_COMPILE_O7
#define OKIIDOKU_COMPILE_O7 false
#endif

#ifndef OKIIDOKU_COMPILE_O8
#define OKIIDOKU_COMPILE_O8 false
#endif

#ifndef OKIIDOKU_COMPILE_O9
#define OKIIDOKU_COMPILE_O9 false
#endif

#ifndef OKIIDOKU_COMPILE_O10
#define OKIIDOKU_COMPILE_O10 true
#endif

#ifndef OKIIDOKU_COMPILE_O11
#define OKIIDOKU_COMPILE_O11 false
#endif

#ifndef OKIIDOKU_COMPILE_O12
#define OKIIDOKU_COMPILE_O12 false
#endif

#ifndef OKIIDOKU_COMPILE_O16
#define OKIIDOKU_COMPILE_O16 true
#endif





// ==================================================================
// !!! contents below are not part of the public config interface !!!


#if OKIIDOKU_COMPILE_O2
#define OKIIDOKU_TEMPLATE_FOR_O2 OKIIDOKU_FOR_COMPILED_O(2)
#else
#define OKIIDOKU_TEMPLATE_FOR_O2
#endif

#if OKIIDOKU_COMPILE_O3
#define OKIIDOKU_TEMPLATE_FOR_O3 OKIIDOKU_FOR_COMPILED_O(3)
#else
#define OKIIDOKU_TEMPLATE_FOR_O3
#endif

#if OKIIDOKU_COMPILE_O4
#define OKIIDOKU_TEMPLATE_FOR_O4 OKIIDOKU_FOR_COMPILED_O(4)
#else
#define OKIIDOKU_TEMPLATE_FOR_O4
#endif

#if OKIIDOKU_COMPILE_O5
#define OKIIDOKU_TEMPLATE_FOR_O5 OKIIDOKU_FOR_COMPILED_O(5)
#else
#define OKIIDOKU_TEMPLATE_FOR_O5
#endif

#if OKIIDOKU_COMPILE_O6
#define OKIIDOKU_TEMPLATE_FOR_O6 OKIIDOKU_FOR_COMPILED_O(6)
#else
#define OKIIDOKU_TEMPLATE_FOR_O6
#endif

#if OKIIDOKU_COMPILE_O7
#define OKIIDOKU_TEMPLATE_FOR_O7 OKIIDOKU_FOR_COMPILED_O(7)
#else
#define OKIIDOKU_TEMPLATE_FOR_O7
#endif

#if OKIIDOKU_COMPILE_O8
#define OKIIDOKU_TEMPLATE_FOR_O8 OKIIDOKU_FOR_COMPILED_O(8)
#else
#define OKIIDOKU_TEMPLATE_FOR_O8
#endif

#if OKIIDOKU_COMPILE_O9
#define OKIIDOKU_TEMPLATE_FOR_O9 OKIIDOKU_FOR_COMPILED_O(9)
#else
#define OKIIDOKU_TEMPLATE_FOR_O9
#endif

#if OKIIDOKU_COMPILE_O10
#define OKIIDOKU_TEMPLATE_FOR_O10 OKIIDOKU_FOR_COMPILED_O(10)
#else
#define OKIIDOKU_TEMPLATE_FOR_O10
#endif

#if OKIIDOKU_COMPILE_O11
#define OKIIDOKU_TEMPLATE_FOR_O11 OKIIDOKU_FOR_COMPILED_O(11)
#else
#define OKIIDOKU_TEMPLATE_FOR_O11
#endif

#if OKIIDOKU_COMPILE_O12
#define OKIIDOKU_TEMPLATE_FOR_O12 OKIIDOKU_FOR_COMPILED_O(12)
#else
#define OKIIDOKU_TEMPLATE_FOR_O12
#endif

#if OKIIDOKU_COMPILE_O16
#define OKIIDOKU_TEMPLATE_FOR_O16 OKIIDOKU_FOR_COMPILED_O(16)
#else
#define OKIIDOKU_TEMPLATE_FOR_O16
#endif


// Can be used to instantiate templates.
// At usage sites, first #define OKIIDOKU_FOR_COMPILED_O and then #undef it right after.
// Note: This _seems_ to violate pitchfork library spec:
//  "A library should not offer the user controls for tweaking its public interface."...
//  https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#libraries
//  But I think it's okay. Each order could be technically be treated as its own standalone API.
#define OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES \
OKIIDOKU_TEMPLATE_FOR_O2 \
OKIIDOKU_TEMPLATE_FOR_O3 \
OKIIDOKU_TEMPLATE_FOR_O4 \
OKIIDOKU_TEMPLATE_FOR_O5 \
OKIIDOKU_TEMPLATE_FOR_O6 \
OKIIDOKU_TEMPLATE_FOR_O7 \
OKIIDOKU_TEMPLATE_FOR_O8 \
OKIIDOKU_TEMPLATE_FOR_O9 \
OKIIDOKU_TEMPLATE_FOR_O10 \
OKIIDOKU_TEMPLATE_FOR_O11 \
OKIIDOKU_TEMPLATE_FOR_O12 \
OKIIDOKU_TEMPLATE_FOR_O16

#endif