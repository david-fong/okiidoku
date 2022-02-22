#ifndef HPP_SOLVENT_CONFIG
#define HPP_SOLVENT_CONFIG
// https://vector-of-bool.github.io/2020/10/04/lib-configuration.html
// #things-that-make-you-wish-c++-was-a-little-more-like-rust-TM (?)
#if __has_include(<solvent_config.tweaks.h>)
	#include <solvent_config.tweaks.h>
#endif

// not that interesting
#ifndef SOLVENT_INCLUDE_ORDER_2
#define SOLVENT_INCLUDE_ORDER_2 false
#endif

// vanilla
#ifndef SOLVENT_INCLUDE_ORDER_3
#define SOLVENT_INCLUDE_ORDER_3 true
#endif

// moar
#ifndef SOLVENT_INCLUDE_ORDER_4
#define SOLVENT_INCLUDE_ORDER_4 true
#endif

// even moarer
#ifndef SOLVENT_INCLUDE_ORDER_5
#define SOLVENT_INCLUDE_ORDER_5 true
#endif

// good luck :)
#ifndef SOLVENT_INCLUDE_ORDER_6
#define SOLVENT_INCLUDE_ORDER_6 false
#endif

// if you have a supercomputer?
#ifndef SOLVENT_INCLUDE_ORDER_7
#define SOLVENT_INCLUDE_ORDER_7 false
#endif

#ifndef SOLVENT_DEFAULT_ORDER
#define SOLVENT_DEFAULT_ORDER 4
#endif

// Whether or not ansi-escape sequences should be emitted within _CLI_
// output to dim out harsh, non-essential text.
#define USE_ANSI_ESC true



// ==================================================================
// !!! contents below are not part of the public config interface !!!

#if (\
   (!SOLVENT_INCLUDE_ORDER_2 && SOLVENT_DEFAULT_ORDER == 2)\
|| (!SOLVENT_INCLUDE_ORDER_3 && SOLVENT_DEFAULT_ORDER == 3)\
|| (!SOLVENT_INCLUDE_ORDER_4 && SOLVENT_DEFAULT_ORDER == 4)\
|| (!SOLVENT_INCLUDE_ORDER_5 && SOLVENT_DEFAULT_ORDER == 5)\
|| (!SOLVENT_INCLUDE_ORDER_6 && SOLVENT_DEFAULT_ORDER == 6)\
|| (!SOLVENT_INCLUDE_ORDER_7 && SOLVENT_DEFAULT_ORDER == 7)\
)
#error cannot default to a non-included order
#endif


#if SOLVENT_INCLUDE_ORDER_2
#define SOLVENT_TEMPL_TEMPL_2_ SOLVENT_TEMPL_TEMPL(2)
#else
#define SOLVENT_TEMPL_TEMPL_2_
#endif
#if SOLVENT_INCLUDE_ORDER_3
#define SOLVENT_TEMPL_TEMPL_3_ SOLVENT_TEMPL_TEMPL(3)
#else
#define SOLVENT_TEMPL_TEMPL_3_
#endif
#if SOLVENT_INCLUDE_ORDER_4
#define SOLVENT_TEMPL_TEMPL_4_ SOLVENT_TEMPL_TEMPL(4)
#else
#define SOLVENT_TEMPL_TEMPL_4_
#endif
#if SOLVENT_INCLUDE_ORDER_5
#define SOLVENT_TEMPL_TEMPL_5_ SOLVENT_TEMPL_TEMPL(5)
#else
#define SOLVENT_TEMPL_TEMPL_5_
#endif
#if SOLVENT_INCLUDE_ORDER_6
#define SOLVENT_TEMPL_TEMPL_6_ SOLVENT_TEMPL_TEMPL(6)
#else
#define SOLVENT_TEMPL_TEMPL_6_
#endif
#if SOLVENT_INCLUDE_ORDER_7
#define SOLVENT_TEMPL_TEMPL_7_ SOLVENT_TEMPL_TEMPL(7)
#else
#define SOLVENT_TEMPL_TEMPL_7_
#endif


// Can be used to instantiate templates.
// Must include `SOLVENT_DEFAULT_ORDER`.
// At usage sites, first #define SOLVENT_TEMPL_TEMPL and then #undef it right after.
// TODO this seems to pitchfork library spec:
//  "A library should not offer the user controls for tweaking its public interface."...
//  https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#libraries
#define SOLVENT_INSTANTIATE_ORDER_TEMPLATES \
SOLVENT_TEMPL_TEMPL_2_ \
SOLVENT_TEMPL_TEMPL_3_ \
SOLVENT_TEMPL_TEMPL_4_ \
SOLVENT_TEMPL_TEMPL_5_ \
SOLVENT_TEMPL_TEMPL_6_ \
SOLVENT_TEMPL_TEMPL_7_



// For internal usage- not for configuration.
#define SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O) T ## O
#define SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, O) SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O)
// Concatenates the value of `T` with the value of `SOLVENT_DEFAULT_ORDER`.
// Use to default-initialize unions with fields with names ending with an order (ex. in toolkit.cpp).
#define SOLVENT_TEMPL_UNION_DEFAULT(T) SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, SOLVENT_DEFAULT_ORDER)


// I wish c++ macros were more powerful :')
// https://c.tenor.com/GdE8Q18Uwp0AAAAC/all-that-for-a-drop-of-blood.gif
#endif