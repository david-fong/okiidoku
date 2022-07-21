include_guard(DIRECTORY)

if(MSVC)
	# add_compile_options("$<$<CONFIG:Debug>:/RTC>")
	# /RTC is incompatible with asan.

	# https://docs.microsoft.com/en-us/cpp/sanitizers/asan
	# https://docs.microsoft.com/en-us/cpp/sanitizers/asan-building
	# known issues: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-known-issues
	#  in particular, see the section "#addresssanitizer-runtime-dll-locations."
	target_compile_options(okiidoku_compile_options_private INTERFACE "$<$<CONFIG:Debug,RelWithDebInfo>:/fsanitize=address>")

	# TODO.try consider trying /sdl (additional security checks)
	#  Note: It has both compile-time and runtime checks, and I only want the runtime checks
	#  for debug-builds, so it seems more suitable to put it here than in the analyzers file.
	#  https://docs.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks?view=msvc-170

else()
	# TODO.wait see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
	#  would this be compatible with sanitizers? or no?

	target_compile_options(okiidoku_compile_options_private INTERFACE "$<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address,undefined>")
	target_link_options(   okiidoku_compile_options_private INTERFACE "$<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address,undefined>")
	# TODO.wait note for empscripten https://emscripten.org/docs/debugging/Sanitizers.html#address-sanitizer (may need to configure increased startup memory)

endif()