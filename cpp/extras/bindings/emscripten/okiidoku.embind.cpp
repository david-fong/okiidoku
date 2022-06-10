// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://emscripten.org/docs/api_reference/bind.h.html
static_assert(__EMSCRIPTEN__);
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
// I think I need to put all these in a same file since EMSCRIPTEN_BINDINGS
// uses static constructor functions and I'm assuming I can't define multiple
// with the same name.

#include <okiidoku/grid.hpp>
#include <okiidoku/shared_rng.hpp>

// TODO.low learn what value types are and see if useful. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

namespace okiidoku {
	void seed_shared_rng(SharedRng& shared_rng, const unsigned long long seed) noexcept {
		shared_rng.rng.seed(seed);
	}
}
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
	namespace oki_m = okiidoku::mono;
	namespace oki_v = okiidoku::visitor;

	// em::constant("sharedRng", oki::shared_rng);
	// TODO.wait currently noexcept functions can't be bound to classes (unintentionally)
	//  https://github.com/emscripten-core/emscripten/pull/15273
	//  https://github.com/emscripten-core/emscripten/pull/17140
	em::class_<oki::SharedRng>("SharedRng")
		// .function("seed", &oki::seed_shared_rng)
		;

	em::class_<oki_v::Grid>("Grid")
		.constructor<oki::Order>()
		// .function("getMonoOrder", &oki::visitor::Grid::get_mono_order) // TODO do I need to define the base class to do this? https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#base-classes
		// .function("atRmi", &oki::visitor::Grid::at_rmi)
		// .function("at", &oki::visitor::Grid::at)
		// .function("followsRule", &oki_v::grid_follows_rule)
		// .function("isFilled",    &oki_v::grid_is_filled)
		;
	em::function("gridFollowsRule", &oki_v::grid_follows_rule);
	em::function("gridIsFilled",    &oki_v::grid_is_filled);
}