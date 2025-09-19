#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
/**
 * Pass --repl to this script to start a REPL.
 * For CommonJS, wrap with `(async() => {...})();`, or `require(".../okiidoku.js").then(oki => {...});`
 * Actual import path for you will be under your install and/or build directory. */
// TODO also try installation directory layout relative path (../../../../bin/okiidoku.js) or relative path for installed example scripts.
const oki = await (await import("./build/Release/okiidoku.js")).default({
	// https://emscripten.org/docs/api_reference/module.html#module
});
// console.log(oki);
oki.rng.seed(BigInt(Date.now()));
{
	using grid = new oki.Grid(3);
	oki.initMostCanonicalGrid(grid);
	oki.generateShuffled(grid, oki.rng.getRngSeed());
	// console.log(grid); // node's default console logging doesn't call toString
	console.log(""+grid); // or use grid.toString() or `${grid}`
	console.log(`value at rmi=0: ${grid.at(0)}`); // or use grid.toString() or `${grid}`
}
if (process.argv.includes("--repl")) {
	const replServer = (await import("node:repl")).start({
		useGlobal: true,
	});
	replServer.context.oki = oki; // could've also globalThis.oki = oki; https://nodejs.org/api/repl.html#global-and-local-scope
}