#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
/**
 * For CommonJS module scripts, you can basically do the same thing as below.
 * You just need to wrap the import await in an async function so it's not top-level.
 * Ex. wrap in an async IIFE: (async() => {...})();
 * Or you can require("./build/Release/okiidoku.js").then(oki => {...});
 */
// console.log(process.cwd());
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
	console.log(`${grid}`); // or use grid.toString() or ""+grid
	grid.delete(); // don't forget to do this. only do it once.
}
if (process.argv.includes("--repl")) {
	const replServer = (await import("node:repl")).start({
		useGlobal: true,
	});
	replServer.context.oki = oki; // could've also globalThis.oki = oki; https://nodejs.org/api/repl.html#global-and-local-scope
}