# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
# cspell:words objfile _M_elems
import dataclasses as dc
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


@dc.dataclass(slots=True)
class IntPrinter:
	def __init__(self, order, sym):
		self.order = order
		self.sym = sym
	def to_string(self):
		# https://stackoverflow.com/a/22798055/11107541
		eval_string = f"(*({self.sym.type}*)({self.sym.address})).val()"
		return gdb.parse_and_eval(eval_string)["_M_elems"]


class O2BitArrPrinter:
	def __init__(self, order, sym):
		# self.word_t_num_bits = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::word_t_num_bits')
		# self.num_words = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::num_words')
		self.order = order
		self.sym = sym
	def to_string(self):
		# https://stackoverflow.com/a/22798055/11107541
		eval_string = f"(*({self.sym.type}*)({self.sym.address})).to_chars()"
		# eval_string = "(*("+str(self.sym.type)+"*)("+str(self.sym.address)+")).to_chars()"
		return gdb.parse_and_eval(eval_string)["_M_elems"]


class MonoGridPrinter:
	def __init__(self, order, sym):
		self.order = order
		self.sym = sym
	def to_string(self):
		return gdb.parse_and_eval(f"okiidoku::mono::print_2d(std::cout, 0, {self.sym})")


def pp_lookup_function(order, sym):
	lookup_tag = sym.type.tag
	if lookup_tag is None:
		return None
	if lookup_tag == "okiidoku::mono::O2BitArr<"+str(order)+">":
		return O2BitArrPrinter(order, sym)
	if lookup_tag == "okiidoku::mono::Grid<"+str(order)+">":
		return MonoGridPrinter(order, sym)
	return None


def register_printers(objfile):
	# print(gdb.parse_and_eval("okiidoku::mono::compiled_orders")) # not possible currently (not exported)
	for order in range(2, 11):
		objfile.pretty_printers.append(lambda x, o=order: pp_lookup_function(o, x))


register_printers(gdb.current_objfile())