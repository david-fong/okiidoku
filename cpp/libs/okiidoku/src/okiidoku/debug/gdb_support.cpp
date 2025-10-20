// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
/**
https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html
	https://sourceware.org/gdb/current/onlinedocs/gdb.html/Which-flavor-to-choose_003f.html
	.debug_gdb_scripts covers the case if okiidoku is built as a static library
note: the "MS" section flags are to remove duplicates.
*/
#if !defined(__EMSCRIPTEN__)
#define DEFINE_GDB_PY_SCRIPT(script_name) \
asm("\
.pushsection \".debug_gdb_scripts\", \"MS\",@progbits,1\n\
.byte 1 /* Python */\n\
.asciz \"" script_name "\"\n\
.popsection \n\
");
// path resolution: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Source-Path.html
DEFINE_GDB_PY_SCRIPT("tools/okiidoku-gdb.py") // add gdb setup command: `directory <path/to/repo/clone>/cpp`
// TODO don't add the above path if building for packaging. probably need to add a compile definition "OKIIDOKU_BUILD_PACKAGING" or something. could add the definition just for this source file.
// DEFINE_GDB_PY_SCRIPT("../share/auto-load/okiidoku-gdb.py")
// DEFINE_GDB_PY_SCRIPT("../share/auto-load/${CMAKE_INSTALL_FULL_LIBDIR}/okiidoku-gdb.py")
#endif