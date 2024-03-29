// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// https://pybind11.readthedocs.io/en/stable/changelog.html
// https://pybind11.readthedocs.io/en/stable/upgrade.html
// https://pybind11.readthedocs.io/en/stable/limitations.html
// https://pybind11.readthedocs.io/en/stable/faq.html
#include <pybind11/pybind11.h>

#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>

namespace py = ::pybind11;

// function called upon python import
PYBIND11_MODULE(okiidoku, m) { //
	namespace oki = ::okiidoku;
	namespace oki_m = ::okiidoku::mono;
	namespace oki_v = ::okiidoku::visitor;

	m.doc() = "pybind11 build of okiidoku";

	// m.def("generate", &add, "generate a random filled sudoku grid");
}