// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://emscripten.org/docs/api_reference/bind.h.html
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
static_assert(__EMSCRIPTEN__);
// I think I need to put all these in a same file since EMSCRIPTEN_BINDINGS
// uses static constructor functions and I'm assuming I can't define multiple
// with the same name.

#include <okiidoku/grid.hpp>

// TODO.low learn what value types are and see if useful. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

namespace okiidoku::mono {
	;
}
namespace okiidoku::visitor {
	;
	// TODO.low define wrappers that return undefined or throw on contract violations / out of bounds. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#non-member-functions-on-the-javascript-prototype
}

// static constructor function:
EMSCRIPTEN_BINDINGS(okiidoku) {
	namespace em = emscripten;
	namespace oki = okiidoku;

	em::class_<oki::visitor::Grid>("Grid")
		.constructor<>()
		.function("getMonoOrder", &oki::visitor::Grid::get_mono_order) // TODO do I need to define the base class to do this? https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#base-classes
		// .function("atRmi", &oki::visitor::Grid::at_rmi)
		// .function("at", &oki::visitor::Grid::at)
		;
	em::function("gridFollowsRule", &oki::visitor::grid_follows_rule);
	em::function("gridIsFilled", &oki::visitor::grid_is_filled);
}