# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words fsanitize, addresssanitizer, asan
include_guard(DIRECTORY)
if(NOT OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS)
	return()
endif()

# setup tips for linking mixed-instrumentation binaries:
#  https://stackoverflow.com/a/47022141/11107541
#  https://github.com/google/sanitizers/wiki/AddressSanitizer#faq

block()
	set(target "okiidoku_compiler_warnings")
	set(configs "$<CONFIG:Debug,RelWithDebInfo>")
	if(MSVC)
		# add_compile_options("$<$<CONFIG:Debug>:/RTC>")
		# /RTC is incompatible with asan.

		# https://docs.microsoft.com/en-us/cpp/sanitizers/asan
		# https://docs.microsoft.com/en-us/cpp/sanitizers/asan-building
		# known issues: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-known-issues
		#  in particular, see the section "#addresssanitizer-runtime-dll-locations."
		target_compile_options("${target}" INTERFACE "$<${configs}:/fsanitize=address>")

		# TODO.try consider trying /sdl (additional security checks)
		#  Note: It has both compile-time and runtime checks, and I only want the runtime checks
		#  for debug-builds, so it seems more suitable to put it here than in the analyzers file.
		#  https://docs.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks?view=msvc-170

	else()
		# TODO.wait see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
		#  would this be compatible with sanitizers? or no?

		target_compile_options("${target}" INTERFACE "$<${configs}:-fsanitize=address,undefined>")
		target_link_options(   "${target}" INTERFACE "$<${configs}:-fsanitize=address,undefined>")
		# TODO note for emscripten https://emscripten.org/docs/debugging/Sanitizers.html#address-sanitizer (may need to configure increased startup memory)

	endif()
endblock()