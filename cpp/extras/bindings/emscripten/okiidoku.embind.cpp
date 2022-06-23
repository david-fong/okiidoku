// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://emscripten.org/docs/api_reference/bind.h.html
// https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md
static_assert(__EMSCRIPTEN__);
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
// I think I need to put all these in a same file since EMSCRIPTEN_BINDINGS
// uses static constructor functions and I'm assuming I can't define multiple
// with the same name.

#include <okiidoku/grid.hpp>

#include <random> // mt19937_64

// TODO.low learn what value types are and see if useful. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

namespace okiidoku {
	class Rng final {
	public:
		std::mt19937_64 rng_;
		void seed(const unsigned long long seed) noexcept {
			rng_.seed(seed);
		}
		[[nodiscard]] std::uint_least64_t get_u64() noexcept {
			return static_cast<std::uint_least64_t>(rng_() - decltype(rng_)::min());
		}
	};
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
	em::class_<oki::Rng>("Rng")
		.function("seed", &oki::Rng::seed)
		.function("getUint64", &oki::Rng::get_u64)
		;

	em::class_<oki_v::Grid>("Grid")
		.constructor<oki::Order>()
		.function("getMonoOrder", &oki::visitor::Grid::get_mono_order) // TODO do I need to define the base class to do this? https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#base-classes
		.function("atRmi", &oki::visitor::Grid::at_rmi)
		.function("at", &oki::visitor::Grid::at)
		.function("followsRule", &oki_v::grid_follows_rule)
		.function("isFilled",    &oki_v::grid_is_filled)
		;
	em::function("gridFollowsRule", &oki_v::grid_follows_rule);
	em::function("gridIsFilled",    &oki_v::grid_is_filled);
}