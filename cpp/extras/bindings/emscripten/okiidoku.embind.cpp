// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://emscripten.org/docs/api_reference/bind.h.html
// https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md
// https://emsettings.surma.technology/
static_assert(__EMSCRIPTEN__);
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
// I think I need to put all these in a same file since EMSCRIPTEN_BINDINGS
// uses static constructor functions and I'm assuming I can't define multiple
// with the same name.

#include <okiidoku/grid.hpp>
#include <okiidoku/gen.hpp>

#include <random> // mt19937_64

// https://github.com/emscripten-core/emscripten/issues/13902

// TODO.low learn what value types are and see if useful. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

namespace okiidoku::em {
	class Rng final {
	public:
		std::mt19937_64 rng_;
		void seed(const std::uint_fast64_t seed) noexcept {
			rng_.seed(seed);
		}
		[[nodiscard]] rng_seed_t get_rng_seed() noexcept {
			return static_cast<rng_seed_t>(rng_() - decltype(rng_)::min());
		}
	};
	Rng rng {};
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

	em::class_<oki::em::Rng>("Rng")
		.function("seed", &oki::em::Rng::seed)
		.function("getRngSeed", &oki::em::Rng::get_rng_seed)
		;
	em::constant("rng", oki::em::rng);

	em::class_<oki_v::Grid>("Grid")
		.constructor<oki::Order>()
		.function("getMonoOrder", &oki::visitor::Grid::get_mono_order) // TODO do I need to define the base class to do this? https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#base-classes
		.function("atRmi", &oki::visitor::Grid::at_rmi)
		.function("at", &oki::visitor::Grid::at)
		.function("followsRule", &oki_v::grid_follows_rule)
		.function("isFilled",    &oki_v::grid_is_filled)
		.function("isEmpty",     &oki_v::grid_is_empty)
		;
	em::function("gridFollowsRule", &oki_v::grid_follows_rule);
	em::function("gridIsFilled",    &oki_v::grid_is_filled);
	em::function("gridIsEmpty",     &oki_v::grid_is_empty);
	em::function("generate",        &oki_v::generate);
}

int main() {}