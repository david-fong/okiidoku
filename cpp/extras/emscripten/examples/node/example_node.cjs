#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
const path = require("path");
const process = require("process");
// const { default: okiidoku_create } = require("./build/Debug/okiidoku");

require("./build/Release/okiidoku")({
	// https://emscripten.org/docs/api_reference/module.html#module
}).then((/**@type {import("./build/Release/okiidoku").MainModule}*/oki) => {
	oki.rng.seed(BigInt(Date.now()));
	{
		const grid = new oki.Grid(3);
		oki.initMostCanonicalGrid(grid);
		oki.generateShuffled(grid, oki.rng.getRngSeed());
		// console.log(grid); // node's default console logging doesn't call toString
		console.log(grid.toString());
		grid.delete();
	}
});