#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// TODO.wait typescript .d.ts generation https://github.com/emscripten-core/emscripten/issues/7083

// console.log(process.cwd());
// TODO why do I need to add JSDoc? I thought/assumed TS would pick up the .d.ts at the same location
/**@type {import("./build/Release/okiidoku").MainModule}*/
const oki = await (await import("./build/Release/okiidoku.js")).default({
	// https://emscripten.org/docs/api_reference/module.html#module
});
// console.log(oki);
oki.rng.seed(BigInt(Date.now()));
{
	const grid = new oki.Grid(3);
	oki.initMostCanonicalGrid(grid);
	oki.generateShuffled(grid, oki.rng.getRngSeed());
	// console.log(grid); // node's default console logging doesn't call toString
	console.log(grid.toString());
	grid.delete();
}