// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// https://nanobind.readthedocs.io/en/latest/basics.html
// https://nanobind.readthedocs.io/en/latest/porting.html#removed
// https://nanobind.readthedocs.io/en/latest/exchanging.html
// https://nanobind.readthedocs.io/en/latest/api_core.html
// https://nanobind.readthedocs.io/en/latest/faq.html#how-can-i-reduce-build-time
#include <nanobind/nanobind.h>

#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>

namespace nb = ::nanobind;

// function called upon python import
NB_MODULE(okiidoku, m) { //
	namespace oki = ::okiidoku;
	namespace oki_m = ::okiidoku::mono;
	namespace oki_v = ::okiidoku::visitor;

	m.doc() = "pybind11 build of okiidoku";

	m.def("generate", &add, "generate a random filled sudoku grid");
}