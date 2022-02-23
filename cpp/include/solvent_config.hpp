#ifndef HPP_SOLVENT_CONFIG
#define HPP_SOLVENT_CONFIG
// https://vector-of-bool.github.io/2020/10/04/lib-configuration.html
// #things-that-make-you-wish-c++-was-a-little-more-like-rust-TM (?)
#if __has_include(<solvent_config.tweaks.h>)
	#include <solvent_config.tweaks.h>
#endif

// apologies for the macros. they're used to selectively expand templates.
// If you have ideas about how to do this better, please open a github issue!

// not that interesting
#ifndef M_SOLVENT_INCLUDE_ORDER_2
#define M_SOLVENT_INCLUDE_ORDER_2 false
#endif

// vanilla
#ifndef M_SOLVENT_INCLUDE_ORDER_3
#define M_SOLVENT_INCLUDE_ORDER_3 true
#endif

// moar
#ifndef M_SOLVENT_INCLUDE_ORDER_4
#define M_SOLVENT_INCLUDE_ORDER_4 true
#endif

// even moarer
#ifndef M_SOLVENT_INCLUDE_ORDER_5
#define M_SOLVENT_INCLUDE_ORDER_5 true
#endif

// good luck :)
#ifndef M_SOLVENT_INCLUDE_ORDER_6
#define M_SOLVENT_INCLUDE_ORDER_6 false
#endif

// if you have a supercomputer?
#ifndef M_SOLVENT_INCLUDE_ORDER_7
#define M_SOLVENT_INCLUDE_ORDER_7 false
#endif

#ifndef M_SOLVENT_DEFAULT_ORDER
#define M_SOLVENT_DEFAULT_ORDER 4
#endif

// Whether or not ansi-escape sequences should be emitted within _CLI_
// output to dim out harsh, non-essential text, etc.
#define USE_ANSI_ESC true



// ==================================================================
// !!! contents below are not part of the public config interface !!!

#if (\
   (!M_SOLVENT_INCLUDE_ORDER_2 && M_SOLVENT_DEFAULT_ORDER == 2)\
|| (!M_SOLVENT_INCLUDE_ORDER_3 && M_SOLVENT_DEFAULT_ORDER == 3)\
|| (!M_SOLVENT_INCLUDE_ORDER_4 && M_SOLVENT_DEFAULT_ORDER == 4)\
|| (!M_SOLVENT_INCLUDE_ORDER_5 && M_SOLVENT_DEFAULT_ORDER == 5)\
|| (!M_SOLVENT_INCLUDE_ORDER_6 && M_SOLVENT_DEFAULT_ORDER == 6)\
|| (!M_SOLVENT_INCLUDE_ORDER_7 && M_SOLVENT_DEFAULT_ORDER == 7)\
)
#error cannot default to a non-included order
#endif


#if M_SOLVENT_INCLUDE_ORDER_2
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 2
#define M_SOLVENT_TEMPL_TEMPL_2_ M_SOLVENT_TEMPL_TEMPL(2)
#else
#define M_SOLVENT_TEMPL_TEMPL_2_
#endif

#if M_SOLVENT_INCLUDE_ORDER_3
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 3
#define M_SOLVENT_TEMPL_TEMPL_3_ M_SOLVENT_TEMPL_TEMPL(3)
#else
#define M_SOLVENT_TEMPL_TEMPL_3_
#endif

#if M_SOLVENT_INCLUDE_ORDER_4
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 4
#define M_SOLVENT_TEMPL_TEMPL_4_ M_SOLVENT_TEMPL_TEMPL(4)
#else
#define M_SOLVENT_TEMPL_TEMPL_4_
#endif

#if M_SOLVENT_INCLUDE_ORDER_5
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 5
#define M_SOLVENT_TEMPL_TEMPL_5_ M_SOLVENT_TEMPL_TEMPL(5)
#else
#define M_SOLVENT_TEMPL_TEMPL_5_
#endif

#if M_SOLVENT_INCLUDE_ORDER_6
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 6
#define M_SOLVENT_TEMPL_TEMPL_6_ M_SOLVENT_TEMPL_TEMPL(6)
#else
#define M_SOLVENT_TEMPL_TEMPL_6_
#endif

#if M_SOLVENT_INCLUDE_ORDER_7
#undef M_SOLVENT_O_MAX
#define M_SOLVENT_O_MAX 7
#define M_SOLVENT_TEMPL_TEMPL_7_ M_SOLVENT_TEMPL_TEMPL(7)
#else
#define M_SOLVENT_TEMPL_TEMPL_7_
#endif


// Can be used to instantiate templates.
// Must include `M_SOLVENT_DEFAULT_ORDER`.
// At usage sites, first #define M_SOLVENT_TEMPL_TEMPL and then #undef it right after.
// TODO this seems to violate pitchfork library spec:
//  "A library should not offer the user controls for tweaking its public interface."...
//  https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#libraries
#define M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES \
M_SOLVENT_TEMPL_TEMPL_2_ \
M_SOLVENT_TEMPL_TEMPL_3_ \
M_SOLVENT_TEMPL_TEMPL_4_ \
M_SOLVENT_TEMPL_TEMPL_5_ \
M_SOLVENT_TEMPL_TEMPL_6_ \
M_SOLVENT_TEMPL_TEMPL_7_



// For internal usage- not for configuration.
#define M_SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O) T ## O
#define M_SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, O) M_SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O)
// Concatenates the value of `T` with the value of `M_SOLVENT_DEFAULT_ORDER`.
// Use to default-initialize unions with fields with names ending with an order (ex. in gen/union.cpp).
#define M_SOLVENT_TEMPL_UNION_DEFAULT(T) M_SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, M_SOLVENT_DEFAULT_ORDER)


// I wish c++ macros were more powerful :')
// https://c.tenor.com/GdE8Q18Uwp0AAAAC/all-that-for-a-drop-of-blood.gif
#endif