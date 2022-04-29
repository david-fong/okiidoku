# String Encodings of Grids

For the purposes of persistence and messaging (emscripten bridge to JS) within this library, binary format (and maybe JSON) are suitable.

If I want to engage in discussions on forums where people use various text string encodings, then I'll need to implement that as well...

The thing is, there are a lot of possible encodings for how to do this and not really a de-facto one. I don't want to litter the library with many serdes implementations. For my own purposes of engaging in community discussions, I'll probably implement some scripts in a tools folder or something for this.