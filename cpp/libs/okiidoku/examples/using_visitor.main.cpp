/**
Everything should be exactly the same as in the mono example,
except that instead of doing

```
namespace okiidoku { using namespace okiidoku::mono; }
okiidoku::Grid<O> grid;
```

do

```
namespace okiidoku { using namespace okiidoku::visitor; }
okiidoku::Grid grid(O);
```
*/