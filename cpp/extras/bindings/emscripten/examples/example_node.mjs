#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// TODO.wait typescript .d.ts generation https://github.com/emscripten-core/emscripten/issues/7083

// console.log(process.cwd());
// import { default as oki }  from "../out/build/dev/Release/okiidoku.js";
globalThis.okiidokuMain = () => {
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
}
const { default: oki } = await import("../out/build/dev/Release/okiidoku.js");