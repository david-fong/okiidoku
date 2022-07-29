#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
const path = require("path");
const process = require("process");

globalThis.okiidokuMain = () => {
	// see file ./example_node.mjs
}
const oki = require(path.join(process.cwd(), "Release/okiidoku.js"));