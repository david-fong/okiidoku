// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://emscripten.org/docs/api_reference/bind.h.html
// https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md
// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#object-ownership
static_assert(__EMSCRIPTEN__); // https://emscripten.org/docs/compiling/Building-Projects.html#detecting-emscripten-in-preprocessor
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>

#include <sstream>
#include <string>
#include <random>  // mt19937_64
#include <cstdint>
#include <type_traits>

// https://github.com/emscripten-core/emscripten/issues/13902 unint64_t is passed through to JS as a signed value instead #13902

// TODO.low learn what value types are and see if useful. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

namespace okiidoku::em { namespace {

	using namespace ::okiidoku::visitor;

	class Rng final {
	public:
		std::mt19937_64 rng_ {};
		void seed(const std::uint_fast64_t seed) noexcept {
			rng_.seed(seed);
		}
		[[nodiscard]] rng_seed_t get() noexcept {
			return static_cast<rng_seed_t>(rng_() - decltype(rng_)::min());
		}
		// auto get_next() noexcept { return rng_(); } // haven't found a need for this
	};
	Rng rng {};

	std::string grid_to_emoji_string(Grid& grid) noexcept {
		std::stringstream ss {};
		print_2d(ss, rng.get(), grid);
		return ss.str();
	}

	void shuffle(Grid& grid) noexcept {
		shuffle(grid, rng.get());
	}
}}
namespace okiidoku::mono {
}
namespace okiidoku::visitor { namespace {
	// TODO.low define wrappers that return undefined or throw on contract violations / out of bounds. https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#non-member-functions-on-the-javascript-prototype
}}

// specify custom marshalling for int wrappers.
// see https://github.com/emscripten-core/emscripten/blob/main/test/embind/test_custom_marshal.cpp
namespace emscripten::internal {

	template<okiidoku::detail::is_int_wrapper T>
	struct BindingType<T> {
		using IntBinding = BindingType<typename T::val_t>;
		using WireType = IntBinding::WireType;
		// C++ -> JS:
		constexpr static WireType toWireType(const T& i, rvp::default_tag) {
			return IntBinding::toWireType(i.val(), rvp::default_tag{});
		}
		// JS -> C++:
		constexpr static T fromWireType(WireType value) {
			// val v {val::take_ownership(value)}; return v.as<T>();
			return T{IntBinding::fromWireType(value)};
		}
	};
	template<okiidoku::detail::is_int_wrapper T>
	struct TypeID<T> {
		static constexpr TYPEID get() {
			return TypeID<typename T::val_t>::get();
		}
	};
}

EMSCRIPTEN_BINDINGS(okiidoku) {
	// (note: ^this is a static constructor function)
	namespace em = ::emscripten;
	namespace oki = ::okiidoku;
	namespace oki_m = ::okiidoku::mono;
	namespace oki_v = ::okiidoku::visitor;

	// em::function("isOrderCompiled", &oki::is_order_compiled);

	em::enum_<oki::IntKind>("IntKind")
		.value("FAST",  oki::IntKind::fast)
		.value("SMALL", oki::IntKind::small)
		;
	em::class_<oki::em::Rng>("Rng")
		.function("seed", &oki::em::Rng::seed)
		.function("getRngSeed", &oki::em::Rng::get)
		;
	em::constant("rng", oki::em::rng);

	em::class_<oki_v::Grid>("Grid")
		.constructor<oki::Order>()
		// .function("getMonoOrder", &oki_v::Grid::get_order) // TODO need to define the base class to do this https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#base-classes
		.function("at",          em::select_overload<oki_v::Grid::sym_t (oki_v::ints::o4x_t rmi                        ) const noexcept>(&oki_v::Grid::operator[]))
		.function("at",          em::select_overload<oki_v::Grid::sym_t (oki_v::ints::o2x_t row, oki_v::ints::o2x_t col) const noexcept>(&oki_v::Grid::operator[]))
		.function("followsRule", &oki_v::grid_follows_rule)
		.function("isFilled",    &oki_v::grid_is_filled)
		.function("isEmpty",     &oki_v::grid_is_empty)
		.function("toString",    &oki::em::grid_to_emoji_string)
		;
	// em::function("gridFollowsRule", &oki_v::grid_follows_rule);
	// em::function("gridIsFilled",    &oki_v::grid_is_filled);
	// em::function("gridIsEmpty",     &oki_v::grid_is_empty);

	em::function("initMostCanonicalGrid", &oki_v::init_most_canonical_grid);
	// TODO change this into an instance method:
	em::function("shuffle", em::select_overload<void (oki_v::Grid&, oki::rng_seed_t rng_seed) noexcept>(&oki_v::generate_shuffled));
	// em::function("generateShuffled", em::select_overload<oki_v::Grid (oki::Order, oki::rng_seed_t) noexcept>(&oki_v::generate_shuffled), em::return_value_policy::take_ownership{});
}
