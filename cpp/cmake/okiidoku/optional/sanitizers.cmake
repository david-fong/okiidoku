# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words fsanitize, addresssanitizer, asan
include_guard(DIRECTORY)
if(NOT OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS)
	return()
endif()
# https://clang.llvm.org/docs/AddressSanitizer.html

# setup tips for linking mixed-instrumentation binaries:
#  https://stackoverflow.com/a/47022141/11107541
#  https://github.com/google/sanitizers/wiki/AddressSanitizer#faq
#  https://github.com/google/sanitizers/wiki/AddressSanitizerAsDso

block()
	set(target "okiidoku_compiler_warnings")
	if(MSVC)
		# add_compile_options("$<$<CONFIG:Debug>:/RTC>") # https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_CHECKS.html
		# /RTC is incompatible with asan.

		# https://docs.microsoft.com/en-us/cpp/sanitizers/asan
		# https://docs.microsoft.com/en-us/cpp/sanitizers/asan-building
		# known issues: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-known-issues
		#  in particular, see the section "#addresssanitizer-runtime-dll-locations."
		target_compile_options("${target}" INTERFACE "$<${debug_configs}:/fsanitize=address>")

		# TODO.try consider trying /sdl (additional security checks)
		#  Note: It has both compile-time and runtime checks, and I only want the runtime checks
		#  for debug-builds, so it seems more suitable to put it here than in the analyzers file.
		#  https://docs.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks?view=msvc-170

	else()
		# TODO.wait see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
		#  would this be compatible with sanitizers? or no?
		set(flags
			"-fsanitize=address,undefined"
			# "$<$<CXX_COMPILER_ID:Clang>:-shared-libasan>"
		)
		target_compile_options("${target}"        INTERFACE "$<${debug_configs}:${flags}>")
		target_link_options(   "${target}" BEFORE INTERFACE "$<${debug_configs}:${flags}>")
		# TODO note for emscripten https://emscripten.org/docs/debugging/Sanitizers.html#address-sanitizer (may need to configure increased startup memory)

	endif()
endblock()
