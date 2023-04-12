# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words objfile _M_elems
import gdb.printing
# https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html
# https://sourceware.org/gdb/onlinedocs/gdb/gdb_002eprinting.html
# https://sourceware.org/gdb/onlinedocs/gdb/Pretty-Printing-API.html
# https://sourceware.org/gdb/onlinedocs/gdb/Python-Auto_002dloading.html
#   https://sourceware.org/gdb/onlinedocs/gdb/Auto_002dloading-extensions.html
#     https://sourceware.org/gdb/onlinedocs/gdb/Which-flavor-to-choose_003f.html
#   https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html
# https://sourceware.org/gdb/onlinedocs/gdb/Values-From-Inferior.html

# https://sourceware.org/gdb/onlinedocs/gdb/Python-Commands.html


class O2BitArrPrinter:
	def __init__(self, order, val):
		self.word_t_num_bits = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+order+'>::word_t_num_bits')
		self.order = order
		self.val = val
	def to_string(self):
		# py print('{:09b}'.format(int(gdb.parse_and_eval('arr')['words_']['_M_elems'][0])))
		# oh... this is actually tricky. especially because I used uint_fast32_t and uint_fast64_t instead of exact sizes
		words = reversed(self.val['words_']['_M_elems'])
		msw_num_bits = self.word_t_num_bits if order == self.word_t_num_bits else (order % self.word_t_num_bits)
		return '{:0'+msw_num_bits+'b}'.format(words[0])
		+ ''.join(map(lambda x: '{:0'+word+'b}'.format(int(x)), words[1:]))


def pp_lookup_function(order, val):
	lookup_tag = val.type.tag
	if lookup_tag is None:
		return None
	if lookup_tag == 'okiidoku::mono::O2BitArr<'+order+'>':
		return O2BitArrPrinter(order, val)
	return None


def register_printers(objfile):
	for order in range(2, 11):
		objfile.pretty_printers.append(order, pp_lookup_function)


register_printers(gdb.current_objfile())