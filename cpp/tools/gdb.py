import gdb.printing
# https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html

class HouseMaskPrinter:
	def __init__(self, val):
		self.val = val
	def to_string(self):
		# return ''.join(map(lambda x: '{64:b}'.format(x), self.val['ints_']['_M_elems']))
		return '' # TODO I have no idea what I'm doing


def okiidoku_build_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter('okiidoku')
	pp.add_printer('HouseMask', '^okiidoku::mono::HouseMask<.*>$', HouseMaskPrinter)
	return pp


gdb.printing.register_pretty_printer(
	gdb.current_objfile(),
	okiidoku_build_pretty_printer()
)