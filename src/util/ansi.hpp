#ifndef HPP_UTIL_ANSI
#define HPP_UTIL_ANSI

namespace Ansi {

    #define SGR(NAME, ON_STR, OFF_STR) constexpr struct {\
        char const*const ON  = ON_STR;\
        char const*const OFF = OFF_STR;\
    } NAME;

    SGR(DIM, "\e[2m", "\e[22m")

    #undef SGR

}; // End of Ansi namespace.

#endif