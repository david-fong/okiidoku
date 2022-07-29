// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
/**
Everything should be exactly the same as in the mono example,
except that instead of doing

```
namespace okiidoku { using namespace ::okiidoku::mono; }
okiidoku::Grid<O> grid;
```

do

```
namespace okiidoku { using namespace ::okiidoku::visitor; }
okiidoku::Grid grid(O);
```
*/