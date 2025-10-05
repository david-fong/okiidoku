# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
# cspell:words objfile _M_elems
import re
import gdb, gdb.printing
# https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html
# https://sourceware.org/gdb/onlinedocs/gdb/gdb_002eprinting.html
# https://sourceware.org/gdb/onlinedocs/gdb/Pretty-Printing-API.html
# https://sourceware.org/gdb/onlinedocs/gdb/Python-Auto_002dloading.html
#   https://sourceware.org/gdb/onlinedocs/gdb/Auto_002dloading-extensions.html
#   https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html
# https://sourceware.org/gdb/onlinedocs/gdb/Values-From-Inferior.html

# https://sourceware.org/gdb/onlinedocs/gdb/Python-Commands.html

# TODO somehow exit if already registered
# > You should write auto-loaded code (see Python Auto-loading) such that it can be evaluated multiple times without changing its meaning
# - https://sourceware.org/gdb/current/onlinedocs/gdb.html/Writing-a-Pretty_002dPrinter.html


class IntPrinter:
	__name_pattern = re.compile(r"^okiidoku::Int<[^>]+?>$")
	def __init__(self, val: gdb.Value):
		self.__max: gdb.Value = val.type.template_argument(0) # type:ignore
		self.__kind = val.type.template_argument(1)
		self.__val = val
	def to_string(self):
		# eval_string = f"(*({self.__val.type}*)({self.__val.address}))"
		# return str(gdb.parse_and_eval(eval_string)["val_"])
		return str(self.__val["val_"])


class O2BitArrPrinter:
	__name_pattern = re.compile(r"^okiidoku::mono::O2BitArr<(?<order>\d+?)>$")
	def __init__(self, val: gdb.Value):
		# self.word_t_num_bits = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::word_t_num_bits')
		# self.num_words = gdb.parse_and_eval('okiidoku::mono::O2BitArr<'+str(order)+'>::num_words')
		self.__order: gdb.Value = val.type.template_argument(0) # type:ignore (my typeshed doesn't have the right signature)
		self.__val = val
	def to_string(self):
		# https://stackoverflow.com/a/22798055/11107541
		eval_string = f"(*({self.__val.type}*)({self.__val.address})).to_chars()"
		# eval_string = "(*("+str(self.__val.type)+"*)("+str(self.__val.address)+")).to_chars()"
		return str(gdb.parse_and_eval(eval_string)["_M_elems"])


class MonoGridPrinter:
	__name_pattern = re.compile(r"^okiidoku::mono::Grid<(?<order>\d+?)>$")
	def __init__(self, val: gdb.Value):
		# self.__order: gdb.Value = val.type.template_argument(0) # type:ignore (my typeshed doesn't have the right signature)
		self.__val = val
	def to_string(self):
		return str(gdb.parse_and_eval(f"okiidoku::mono::print_2d(std::cout, 0, {self.__val})"))


def pp_lookup_function(val: gdb.Value):
	type_name = val.type.name
	if type_name is None:
		return None
	for printer in (IntPrinter, O2BitArrPrinter, MonoGridPrinter):
		if printer.__name_pattern.match(type_name):
			return printer(val)
	return None


def register_printers(objfile: gdb.Objfile | None):
	if (objfile is None):
		return
	if (objfile.filename is not None and "okiidoku" not in objfile.filename):
		return
	# print(gdb.parse_and_eval("okiidoku::mono::compiled_orders")) # not possible currently (not exported)
	objfile.pretty_printers.append(pp_lookup_function)


register_printers(gdb.current_objfile()) # https://sourceware.org/gdb/current/onlinedocs/gdb.html/Objfiles-In-Python.html#Objfiles-In-Python