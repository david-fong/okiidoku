# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
# cspell:words objfile _M_elems
import typing, re
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


def _call_member_fn(val: gdb.Value, fn_name: str, *args):
	# https://stackoverflow.com/a/22798055/11107541
	# 17.5.1 "Calling functions with no debug info" https://sourceware.org/gdb/current/onlinedocs/gdb.html/Calling.html
	# ret_v = val[fn_name]() # <- we want to do this, but GDB isn't cooperating :(
	ret_t  = val[fn_name].type.target()
	val_pt = val.type.strip_typedefs().pointer()
	ret_v  = gdb.parse_and_eval(f"({ret_t})((({val_pt}){val.address})->{fn_name}({', '.join(args)}))")
	return ret_v


class IntPrinter:
	__name_pattern = re.compile(r"^okiidoku::Int<[^>]+?>$")
	def __init__(self, val: gdb.Value):
		self.__val = val
	def to_string(self):
		v = self.__val
		val_ = v["val_"].cast(gdb.lookup_type("unsigned long long"))
		k_ = str(v["kind"]).split("::")[-1][0]
		return f"{val_}/{v['max']}{k_}"


class BitArrayPrinter(gdb.ValuePrinter):
	__name_pattern = re.compile(r"^okiidoku::BitArray<(?P<width>[^>]+?),\s*?(?P<kind>[^>]+?)>$")
	def __init__(self, val: gdb.Value):
		self.__val = val
		self.__width = int(val.type.template_argument(0)) # type:ignore (typeshed signature)
		# self.__kind:  gdb.Value = val.type.template_argument(1) # type:ignore (typeshed signature)
	def to_string(self):
		v = self.__val
		# count = _call_member_fn(v,"count")["_M_elems"]
		chars = _call_member_fn(v,"to_chars")["_M_elems"]
		# TODO respect `gdb.print_options()['max_elements']` ?
		return f"{self.__width}:{chars}"
	# def display_hint(self):
	# 	return "array"
	# def num_children(self):
	# 	return self.__width
	# def child(self, n: int):
	# 	bit = _call_member_fn(self.__val, "operator[]", n)
	# 	return (str(n), bit)
	# def children(self):
	# 	return (self.child(n) for n in range(self.num_children()))


class MonoGridPrinter(gdb.ValuePrinter):
	__name_pattern = re.compile(r"^okiidoku::mono::Grid<(?P<order>[^>]+?)>$")
	def __init__(self, val: gdb.Value):
		self.__order: gdb.Value = val.type.template_argument(0) # type:ignore (typeshed signature)
		self.__size = int(self.__order) ** 4
		self.__val = val
	def to_string(self):
		return f"Grid<{int(self.__order)}>"
		# return str(gdb.parse_and_eval(f"okiidoku::mono::print_2d(std::cout, 0, {self.__val})"))
	def display_hint(self):
		return "array"
	def num_children(self):
		return self.__size
	def child(self, n: int):
		v = self.__val
		O2 = int(self.__order ** 2)
		return (f"{n//O2},{n%O2}", v["arr"][n])
	def children(self):
		return (self.child(n) for n in range(self.num_children()))


def pp_lookup_function(val: gdb.Value):
	type_name = val.type.strip_typedefs().name
	if type_name is None:
		return None
	for printer in (
		IntPrinter,
		BitArrayPrinter,
		MonoGridPrinter,
	):
		if getattr(printer, f"_{printer.__name__}__name_pattern").match(type_name):
			return printer(val)
	return None


def register_printers(objfile: gdb.Objfile | None):
	if (objfile is None):
		return
	if (objfile.filename is not None and "okiidoku" not in objfile.filename):
		gdb.write(f"not loading okiidoku-gdb.py for {objfile.filename}", gdb.STDOUT)
		return
	# gdb.write(gdb.parse_and_eval("okiidoku::mono::compiled_orders"), gdb.STDOUT) # not possible currently (not exported)
	objfile.pretty_printers.append(pp_lookup_function)


register_printers(gdb.current_objfile()) # https://sourceware.org/gdb/current/onlinedocs/gdb.html/Objfiles-In-Python.html#Objfiles-In-Python
gdb.write(f"successfully loaded okiidoku-gdb.py for {gdb.current_objfile()}\n", gdb.STDOUT)