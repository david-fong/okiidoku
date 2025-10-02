# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
# cspell:words objfile _M_elems
import dataclasses as dc
import gdb, gdb.printing
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
	def __init__(self, order, val: gdb.Value):
		self.order = order
		self.__val = val
	def to_string(self):
		# https://stackoverflow.com/a/22798055/11107541
		eval_string = f"(*({self.__val.type}*)({self.__val.address})).val()"
		return gdb.parse_and_eval(eval_string)["_M_elems"]


class O2BitArrPrinter:
	def __init__(self, order, val: gdb.Value):
		# self.word_t_num_bits = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::word_t_num_bits')
		# self.num_words = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::num_words')
		self.order = order
		self.__val = val
	def to_string(self):
		# https://stackoverflow.com/a/22798055/11107541
		eval_string = f"(*({self.__val.type}*)({self.__val.address})).to_chars()"
		# eval_string = "(*("+str(self.__val.type)+"*)("+str(self.__val.address)+")).to_chars()"
		return gdb.parse_and_eval(eval_string)["_M_elems"]


class MonoGridPrinter:
	def __init__(self, order, val: gdb.Value):
		self.order = order
		self.__val = val
	def to_string(self):
		return gdb.parse_and_eval(f"okiidoku::mono::print_2d(std::cout, 0, {self.__val})")


def pp_lookup_function(order, val: gdb.Value):
	# TODO instead of taking order as an argument, get it from val.type.template_argument (https://sourceware.org/gdb/current/onlinedocs/gdb.html/Types-In-Python.html#Types-In-Python)
	lookup_tag = val.type.tag
	if lookup_tag is None:
		return None
	if lookup_tag == f"okiidoku::mono::O2BitArr<{order}>":
		return O2BitArrPrinter(order, val)
	if lookup_tag == f"okiidoku::mono::Grid<{order}>":
		return MonoGridPrinter(order, val)
	return None


def register_printers(objfile: gdb.Objfile):
	# print(gdb.parse_and_eval("okiidoku::mono::compiled_orders")) # not possible currently (not exported)
	for order in range(2, 11):
		objfile.pretty_printers.append(lambda x, o=order: pp_lookup_function(o, x))


register_printers(gdb.current_objfile())