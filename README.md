# Modern Half-Life

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![Status](https://img.shields.io/badge/status-active-success)
![License](https://img.shields.io/badge/license-Valve_SDK-lightgrey)

Modern Half-Life is an effort to bring the original Half-Life 1 SDK into the modern C++ era.

The primary objective is to migrate the codebase to **C++23** while preserving the original behavior of the game. This is a modernization project, not a remake, rewrite, or gameplay overhaul. If a change would alter how Half-Life behaves, preserving the original implementation takes priority.

## Why?

The original Half-Life SDK was released over two decades ago and reflects the programming practices of its time. While the code remains historically significant, it can be difficult to work with using modern compilers and tooling.

Modern Half-Life aims to provide a clean, modern foundation that developers can comfortably build upon without first spending days or weeks updating a late 1990s codebase.

Whether you're interested in creating a mod, experimenting with engine programming, or simply studying the SDK, the goal is to let you start with modern C++ instead of legacy C/C++.

## Project Goals

This project includes changes such as:

* Migrating the project to **C++23**.
* Replacing legacy C idioms with modern C++ equivalents where appropriate.
* Improving type safety and const-correctness.
* Removing obsolete language features and compiler workarounds.
* Cleaning up warnings and improving compatibility with modern toolchains.
* Improving readability, maintainability, and consistency.
* Using the C++ standard library where it provides clear benefits.

Every modernization is intended to preserve runtime behavior unless explicitly documented otherwise.

## What This Project Is Not

Modern Half-Life is **not** intended to:

* Change gameplay.
* Introduce new engine features.
* Rebalance or redesign existing systems.
* Turn Half-Life into a different game.

Those are better suited for forks and mods built on top of this project.

## For Mod Developers

One of the main goals of this repository is to become a modern starting point for Half-Life development.

Instead of beginning with a 1998 codebase and modernizing it yourself, you can fork this repository and focus on building your game or mod immediately.

If this project succeeds, future developers should spend less time fighting outdated code and more time creating new experiences.

## Contributing

Contributions are welcome as long as they align with the project's philosophy:

* Preserve original behavior whenever possible.
* Prefer modern C++23 features over legacy constructs.
* Improve readability and maintainability.
* Keep changes well-documented and easy to review.

## License

This repository is based on Valve's Half-Life SDK.

Ownership of the original source code remains with Valve Corporation and is distributed under the terms of the Half-Life SDK License. See `LICENSE.txt` and `third_party_licenses.txt` for the original licensing terms.

This repository contains modernization work on top of the original SDK and is intended to make the codebase easier to maintain and extend while respecting the original license.
