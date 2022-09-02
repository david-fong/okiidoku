#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
const path = require("path");
const process = require("process");
// const { default: okiidoku_create } = require("./build/Debug/okiidoku");

require("./build/Debug/okiidoku")({
	// https://emscripten.org/docs/api_reference/module.html#module
}).then((oki) => {
	// console.log(oki);
});