# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# https://sourceware.org/gdb/current/onlinedocs/gdb.html/gdbinit-man.html
# https://sourceware.org/gdb/current/onlinedocs/gdb.html/Init-File-in-the-Current-Directory.html#Init-File-in-the-Current-Directory
# https://sourceware.org/gdb/current/onlinedocs/gdb.html/Auto_002dloading-safe-path.html#Auto_002dloading-safe-path

set auto-load python-scripts on # on by default
# info auto-load python-scripts

set print pretty on
set print array on
set print array-indexes on
set print static-members off

# set substitute-path /okiidoku :/cpp # done in settings.json
set breakpoint pending on
b __sanitizer::Die # https://github.com/google/sanitizers/wiki/AddressSanitizerAndDebugger#gdb

skip -rfu ^Catch::