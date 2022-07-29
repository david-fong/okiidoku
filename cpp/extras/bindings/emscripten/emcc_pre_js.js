// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

// https://emscripten.org/docs/api_reference/module.html#module
// Module["preInit"] = [() => {}];
// Module["preRun"] = [() => {}];
Module["onRuntimeInitialized"] = function() { okiidokuMain(); };