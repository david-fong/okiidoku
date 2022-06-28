#!/usr/bin/env node
const path = require("path");
const process = require("process");

globalThis.okiidokuMain = () => {
	// console.log(oki);
	oki.rng.seed(BigInt(Date.now()));
	const grid = new oki.Grid(3);
	oki.generate(grid, oki.rng.getRngSeed());
}
const oki = require(path.join(process.cwd(), "Release/okiidoku.js"));