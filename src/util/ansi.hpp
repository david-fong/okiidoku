#ifndef HPP_UTIL_ANSI
#define HPP_UTIL_ANSI

/**
 * Yes, I could get some library for this, or I could write a whole
 * library for this, but I'd rather cut down on bloat and dependency
 * since the portion of a full-library that I want is very small.
 */
namespace Ansi {

    #if USE_ANSI_ESC
    #define SGR(NAME, ON_STR, OFF_STR) constexpr struct {\
        char const*const ON  = ON_STR;\
        char const*const OFF = OFF_STR;\
    } NAME;
    #else
    #define SGR(NAME, ON_STR, OFF_STR) constexpr struct {\
        char const*const ON  = "";\
        char const*const OFF = "";\
    } NAME;
    #endif

    SGR(DIM, "\e[2m",  "\e[22m")
    SGR(RED, "\e[31m", "\e[39m")

    #undef SGR

}; // End of Ansi namespace.

#endif