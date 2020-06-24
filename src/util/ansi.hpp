#ifndef HPP_UTIL_ANSI
#define HPP_UTIL_ANSI

#include <array>

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


    // Do not attempt to create filled-strings using entries from this
    // array. The result will not be as expected, which may have to do
    // with the use of utf-8 strings...
    const std::array<std::string, 4> GREYSCALE_BLOCK_CHARS = {
        #if USE_ANSI_ESC
        // NOTE: Make sure that the initializer list size matches that
        // of the corresponding template argument. Compilers won't warn.
        // See https://cppreference.com/w/cpp/language/sizeof...#Example
        // for an example utility function I can make to avoid this problem.
        u8"\u2591", u8"\u2592", u8"\u2593", u8"\u2588",
        #else
        "-", "*", "X", "#"
        #endif
    };

}; // End of Ansi namespace.

#endif