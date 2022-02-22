#ifndef HPP_SOLVENT_CONFIG
#define HPP_SOLVENT_CONFIG
// https://vector-of-bool.github.io/2020/10/04/lib-configuration.html

// Whether or not ansi-escape sequences should be emitted within _CLI_
// output to dim out harsh, non-essential text.
#define USE_ANSI_ESC true

#define SOLVENT_DEFAULT_ORDER 4



// ==================================================================
// !!! contents below are not part of the public config interface !!!

// Can be used to instantiate templates.
// Must include `SOLVENT_DEFAULT_ORDER`.
// At usage sites, first #define SOLVENT_TEMPL_TEMPL and then #undef it right after.
// TODO this seems to pitchfork library spec:
//  "A library should not offer the user controls for tweaking its public interface."...
//  https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#libraries
#define SOLVENT_INSTANTIATE_ORDER_TEMPLATES \
SOLVENT_TEMPL_TEMPL(3) \
SOLVENT_TEMPL_TEMPL(4) \
SOLVENT_TEMPL_TEMPL(5)


// For internal usage- not for configuration.
#define SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O) T ## O
#define SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, O) SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O)
// Concatenates the value of `T` with the value of `SOLVENT_DEFAULT_ORDER`.
// Use to default-initialize unions with fields with names ending with an order (ex. in toolkit.cpp).
#define SOLVENT_TEMPL_UNION_DEFAULT(T) SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, SOLVENT_DEFAULT_ORDER)

#endif