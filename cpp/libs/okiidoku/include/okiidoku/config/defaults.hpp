#ifndef HPP_OKIIDOKU__CONFIG__DEFAULTS
#define HPP_OKIIDOKU__CONFIG__DEFAULTS

// https://vector-of-bool.github.io/2020/10/04/lib-configuration.html
#if __has_include(<okiidoku_compiled_orders.tweaks.h>)
	#include <okiidoku_compiled_orders.tweaks.h>
#endif

// apologies for the macros. they're used to selectively instantiate templates,
// which allow allocating frequently-used data on the stack instead of the heap.

// unfortunately we can't use the constexpr variable/function flavour of
// tweak configs since they aren't accessible to the preprocessor.
#ifndef OKIIDOKU_OPTION_SUPPORT_O2
#define OKIIDOKU_OPTION_SUPPORT_O2 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O3
#define OKIIDOKU_OPTION_SUPPORT_O3 true
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O4
#define OKIIDOKU_OPTION_SUPPORT_O4 true
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O5
#define OKIIDOKU_OPTION_SUPPORT_O5 true
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O6
#define OKIIDOKU_OPTION_SUPPORT_O6 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O7
#define OKIIDOKU_OPTION_SUPPORT_O7 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O8
#define OKIIDOKU_OPTION_SUPPORT_O8 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O9
#define OKIIDOKU_OPTION_SUPPORT_O9 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O10
#define OKIIDOKU_OPTION_SUPPORT_O10 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O11
#define OKIIDOKU_OPTION_SUPPORT_O11 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O12
#define OKIIDOKU_OPTION_SUPPORT_O12 false
#endif

#ifndef OKIIDOKU_OPTION_SUPPORT_O16
#define OKIIDOKU_OPTION_SUPPORT_O16 false
#endif

#endif