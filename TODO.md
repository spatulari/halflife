# TODO

This document serves as a central index of all project TODOs, making it easy to locate and track outstanding work.

## TODO Categories

### `TODO-XXX`

Numbered TODOs that are referenced directly in the source code. This allows you to quickly locate a specific task using your IDE's search functionality (e.g. searching for `// TODO-001` or `// TODO-021`).

### `TODO-A`

General project-wide tasks that are not tied to a specific location in the codebase. These typically involve large-scale refactoring or coding standards, such as removing Hungarian notation or renaming identifiers to follow PascalCase.

## TODO List

* **TODO-001:** Refactor `DBG_AssertFunction` to use `std::source_location` instead of relying on the `__FILE__` and `__LINE__` macros.
