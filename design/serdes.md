# Serialization and Deserialization

For the purposes of persistence and messaging (emscripten bridge to JS) within this library, binary format (as opposed to utf8) is suitable.

If I want to engage in discussions on forums where people use text strings, then I'll need to implement that as well.

The thing is, there are a lot of possible "standards" for how to do this serdes and not really a de-facto one. I'm sure if people want to use this library for their own projects, eventually at least one of those people will want something related to this. I don't want to litter the library with many serdes implementations so I probably won't give into any such requests. But for my own purposes of engaging in community discussions I'll probably implement some scripts in a tools folder or something for this.