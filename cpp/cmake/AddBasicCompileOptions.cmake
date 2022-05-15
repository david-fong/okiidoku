# settings to enable standard compliance and enable many warnings
# this should help ensure that this library supports most compilers
# and isn't less strict on itself than a very strict user's compiler
# options might be on it.

# source file and compiler option parsing rules:
if(MSVC)
	add_compile_options(
		/options:strict # unrecognized compiler options are errors
		/utf-8        # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/wd5030       # warning disable: "unrecognized attribute". (unfortunately needed or else MSVC warns for gnu-prefixed attributes)
	)
else()
	add_compile_options(
		# -Wbidi-chars=any # warn on any usage of bidi text
		# -Wnormalized # warn on identifiers that look the same but are not the same
	)
endif()


# diagnostics formatting:
if(MSVC)
	add_compile_options(
		/diagnostics:caret
	)
else()
	add_compile_options(
	)
endif()


# standards compliance and abi:
if(MSVC)
	add_compile_options(
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/EHsc         # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
	)
else()
	add_compile_options(
		# -Wabi
	)
endif()


# warnings:
if(MSVC)
	add_compile_options(
		/W4 #/WX      # highest warning level #and treat warnings as errors.
	)
else()
	add_compile_options(
		-Wall -Wextra -Wpedantic #-Werror
		-Wfatal-errors # stop compilation on first error. I found it hard to read multiple.
		-Wold-style-cast -Wcast-qual -Wcast-align
		-Wformat=2
		-Wconversion # -Wsign-conversion -Warith-conversion
		-Wshadow -Woverloaded-virtual
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		add_compile_options(
			-Wimplicit-fallthrough
		)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		add_compile_options(
			-Wuseless-cast
			-Wimplicit-fallthrough=5
			# TODO.try
			# -Wvla # maybe put this in the project-root cmake file
			# -Wpedantic-errors
			# -Walloc-zero
			# -Wenum-compare -Wenum-conversion
			# -Wduplicated-cond
			# -Wlogical-op
			# -Wmissing-declarations
			# -Wunused-macros
			# -Wundef # warn on undefined identifier used in `#if`
			# -Wtrampolines
			# -Wframe-larger-than=byte-size -Wstack-usage=byte-size
			# -fno-nonansi-builtins
			# -Wno-builtin-declaration-mismatch
			# -Wunsafe-loop-optimizations
			# -fno-implicit-templates or -frepo # https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Template-Instantiation.html#Template-Instantiation

			# interesting but probably too overboard:
			# -Wpadded
			# -Wdisabled-optimization
			# -Wdouble-promotion
		)
	endif()
endif()