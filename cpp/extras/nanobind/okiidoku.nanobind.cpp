// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// https://nanobind.readthedocs.io/en/latest/basics.html
// https://nanobind.readthedocs.io/en/latest/porting.html#removed
// https://nanobind.readthedocs.io/en/latest/exchanging.html
// https://nanobind.readthedocs.io/en/latest/api_core.html
// https://nanobind.readthedocs.io/en/latest/faq.html#how-can-i-reduce-build-time
// https://nanobind.readthedocs.io/en/latest/packaging.html
#include <nanobind/nanobind.h>

#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>

namespace nb = ::nanobind;
using namespace nb::literals;

// function called upon python import
// https://nanobind.readthedocs.io/en/latest/api_core.html#c.NB_MODULE
// https://nanobind.readthedocs.io/en/latest/basics.html
NB_MODULE(okiidoku_nb, m) { // NOLINT(performance-unnecessary-value-param) https://github.com/wjakob/nanobind/blob/master/docs/faq.rst#policy-on-clang-tidy--wpedantic-etc
	namespace oki = ::okiidoku;
	namespace oki_m = ::okiidoku::mono;
	namespace oki_v = ::okiidoku::visitor;

	m.doc() = "nanobind build of okiidoku";

	nb::class_<oki_v::Grid>(m, "Grid")
		// .def(nb::init<>())
		// .def(nb::init<oki_v::Grid&>())
		;

	// m.def("generate_shuffled", static_cast<void(*)(oki_v::Grid&, oki_m::rng_seed_t)>(&oki_v::generate_shuffled), "generate a random filled sudoku grid");
}