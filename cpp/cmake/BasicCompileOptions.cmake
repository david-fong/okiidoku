include_guard(DIRECTORY)

# settings to enable standard compliance and enable many warnings
# this should help ensure that this library supports most compilers
# and isn't less strict on itself than a very strict user's compiler
# options might be on it.

# add_library(okiidoku_compile_options_public  INTERFACE IMPORTED)
add_library(okiidoku_compile_options_private INTERFACE IMPORTED)

# source file and compiler option parsing rules:
if(MSVC)
	target_compile_options(okiidoku_compile_options_private INTERFACE
		/options:strict # unrecognized compiler options are errors
		/utf-8  # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/wd5030 # warning disable: "unrecognized attribute"
		/wd4068 # warning disable: "unrecognized pragma"
		# TODO 5030 and utf-8 may not work as a private-only compiler option... we can either make it a public
		# otpion, or use https://docs.microsoft.com/en-us/cpp/preprocessor/warning?view=msvc-170
	)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(okiidoku_compile_options_private INTERFACE
		# -Wbidi-chars=any # warn on any usage of bidi text
		-Wnormalized # warn on identifiers that look the same but are not the same
	)
endif()


# diagnostics formatting:
if(okiidoku_IS_TOP_LEVEL)
	if(MSVC)
		target_compile_options(okiidoku_compile_options_private INTERFACE
			/diagnostics:caret
		)
	else()
		target_compile_options(okiidoku_compile_options_private INTERFACE
		)
	endif()
endif()


# standards compliance and abi:
if(MSVC)
	target_compile_options(okiidoku_compile_options_private INTERFACE
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
	)
else()
	target_compile_options(okiidoku_compile_options_private INTERFACE
		# -Wabi
	)
endif()


# warnings:
if(MSVC)
	target_compile_options(okiidoku_compile_options_private INTERFACE
		/W4 # highest warnings level
	)
else()
	target_compile_options(okiidoku_compile_options_private INTERFACE
		-Wall -Wextra -Wpedantic -pedantic-errors
		-Wfatal-errors # stop compilation on first error. I found it hard to read multiple.
		-Wold-style-cast
		-Wvla # maybe put this in the project-root cmake file
		-Wconversion -Wsign-conversion #-Warith-conversion
		-Wdouble-promotion
		# -Wframe-larger-than=byte-size -Wstack-usage=byte-size

		-Wdisabled-optimization # not an error. notification that function is too big. gcc gave up on optimizing.

		-Wformat=2
		-Wcast-qual -Wcast-align
		-Wshadow -Woverloaded-virtual
		-Wnon-virtual-dtor
		# -Wnoexcept
		-Wenum-compare -Wenum-conversion
		-Wmissing-declarations
		-Wunused-macros
		-Wundef # warn on undefined identifier used in `#if`
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR EMSCRIPTEN)
		target_compile_options(okiidoku_compile_options_private INTERFACE
			-Wimplicit-fallthrough
		)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		target_compile_options(okiidoku_compile_options_private INTERFACE
			-Wuseless-cast
			-Wimplicit-fallthrough=5
			-Walloc-zero
			-Wlogical-op
			-Wduplicated-cond

			-Wsuggest-attribute=pure
			-Wsuggest-attribute=const
			-Wsuggest-attribute=noreturn
			# TODO.try
			# -fno-implicit-templates or -frepo # https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Template-Instantiation.html#Template-Instantiation

			-Wno-builtin-declaration-mismatch
			-fno-nonansi-builtins # Disable built-in declarations of functions that are not mandated by ANSI/ISO C. These include ffs, alloca, _exit, index, bzero, conjf, and other related functions.
			-Wtrampolines

			# -Wunsafe-loop-optimizations # only meaningful with -funsafe-loop-optimizations


			# interesting but probably too overboard:
			# -Wpadded
			# -Wsign-promo # actually warns on overload selection behaviour mandated by the standard :O
		)
	endif()
endif()