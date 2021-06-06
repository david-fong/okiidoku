#ifndef HPP_SOLVENT_BUILDFLAG
#define HPP_SOLVENT_BUILDFLAG

// Whether or not ansi-escape sequences should be emitted within output
// to dim out harsh, non-essential text.
#define USE_ANSI_ESC true

// Whether or not to enable windows ansi escape code handling.
#define WINDOWS_ANSI (USE_ANSI_ESC && false)

// Can be used to instantiate templates.
// At usage sites, first #define SOLVENT_TEMPL_TEMPL and then #undef it right after.
#define SOLVENT_INSTANTIATE_ORDER_TEMPLATES \
SOLVENT_TEMPL_TEMPL(2) \
SOLVENT_TEMPL_TEMPL(3) \
SOLVENT_TEMPL_TEMPL(4) \
SOLVENT_TEMPL_TEMPL(5) \
SOLVENT_TEMPL_TEMPL(6)

#endif