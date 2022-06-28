// https://emscripten.org/docs/api_reference/module.html#module
// Module["preInit"] = [() => {}];
// Module["preRun"] = [() => {}];
Module["onRuntimeInitialized"] = function() { okiidokuMain(); };