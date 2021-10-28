#ifndef HPP_SOLVENT_BUILDFLAG
#define HPP_SOLVENT_BUILDFLAG

// Whether or not ansi-escape sequences should be emitted within output
// to dim out harsh, non-essential text.
#define USE_ANSI_ESC true

// Whether or not to enable windows ansi escape code handling.
#define WINDOWS_ANSI (USE_ANSI_ESC && false)

#define SOLVENT_DEFAULT_ORDER 4

// Can be used to instantiate templates.
// Must include `SOLVENT_DEFAULT_ORDER`.
// At usage sites, first #define SOLVENT_TEMPL_TEMPL and then #undef it right after.
#define SOLVENT_INSTANTIATE_ORDER_TEMPLATES \
SOLVENT_TEMPL_TEMPL(3) \
SOLVENT_TEMPL_TEMPL(4) \
SOLVENT_TEMPL_TEMPL(5)


// For internal usage- not for configuration.
#define SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O) T ## O
#define SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, O) SOLVENT_TEMPL_UNION_DEFAULT__PASTER(T, O)
#define SOLVENT_TEMPL_UNION_DEFAULT(T) SOLVENT_TEMPL_UNION_DEFAULT__EVALUATOR(T, SOLVENT_DEFAULT_ORDER)

#endif