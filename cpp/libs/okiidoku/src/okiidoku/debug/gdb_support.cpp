// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
// https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html
// if not this, then https://sourceware.org/gdb/current/onlinedocs/gdb.html/Python-Auto_002dloading.html#Python-Auto_002dloading
/* Note: The "MS" section flags are to remove duplicates. */
#define DEFINE_GDB_PY_SCRIPT(script_name) \
	asm("\
.pushsection \".debug_gdb_scripts\", \"MS\",@progbits,1\n\
.byte 1 /* Python */\n\
.asciz \"" script_name "\"\n\
.popsection \n\
");
DEFINE_GDB_PY_SCRIPT("share/okiidoku.gdb.py") // matches installation location