---
parent: Developer Documentation
nav_order: 2
---
# Project Overview

## Introduction

### Purpose
This document provides a high-level overview of the structure of the Dissolve project, its language and associated build systems, and source tree layout.

### Scope
This overview provides basic information suitable for developers wishing to build and contribute to the project, spanning on the coding language, executable targets, build systems, external dependencies and source tree layout.

## Basics

### Project Purpose
Dissolve is a code to permit interrogation of scattering data through the classical simulation of underlying experimental systems. For more of an overview see the [project description](description.md) document.

### Language
Dissolve is written entirely in C++. For relevant conventions and guidelines regarding style see the [coding standards](standards.md) document.

## Build System

Both cmake and autotools (Makefile) build systems are used in order to provide cross-platform compatibility. The integrity of both must be maintained when modifying/adding files associated to the project.

### Executable Targets
- `dissolve-serial`: Serial CLI version.
- `dissolve`: (Optional) Parallel CLI version (requires suitable MPI compiler).
- `dissolve-gui`: (Optional) Graphical user interface for Dissolve.

Note that binaries are named as listed above only on Linux and OSX. Windows binaries are built as `Dissolve-Serial`, `Dissolve`, and `Dissolve-GUI`.

### External Dependencies
- `dissolve-serial`: Bison (required for construction of various parser-lexers).
- `dissolve`: Suitable MPI library (e.g. OpenMPI 1.10+).
- `dissolve-gui`: Qt 5 (Qt5Gui, Qt5OpenGL, Qt5Widgets, and Qt5PrintSupport), Freetype2, FTGL.

### AutoTools (Makefile)
After running `./autogen.sh`, running `./configure` with no additional arguments will build only `dissolve-serial`. Options to controls the build otherwise are as follows:
- `--with-checks`: Enable additional safety checking throughout the code (e.g. bounds checking), controlled by `#ifdef CHECKS` directives. Recommended to enable for development, and disable for production runs.
- `--with-gui`: Enable building of `dissolve-gui`, alongside `dissolve-serial`.
- `--with-parallel`: Enable building of `dissolve` in parallel with MPI. Note that this disables building of the `dissolve-gui` and `dissolve-serial`. The MPI compiler to use may be specified as an optional argument (e.g. `--with-parallel=icc`), otherwise `mpic++` is assumed.

### CMake
Use `-DCHECKS=1`, `-DGUI=1`, or `-DPARALLEL=1` to enable the options described above in the CMake build.

## Source Tree Layout
All source files are contained within the `src/` directory which is laid out as follows:

### Files
- `dissolve-gui.cpp`: Main entry point for GUI.
- `main.cpp`: Main entry point for serial and parallel versions (determined via the `PARALLEL` define).
- `version.h`: Header containing version information.

### Sub-directories
- `analyse`: Core classes used in the piece-wise construction of calculation pathways in relevant analysis modules.
- `base`: Classes providing general functionality that is free of context from Dissolve and its intended application.
- `classes`: Classes providing core contextual objects used in the operation of Dissolve, and which depend only on other classes in the same directory or those in `base/`.
- `data`: Objects providing static data such as the periodic table, scattering lengths, forcefield parameters etc.
- `expression`: Mathematical expression parser / generator.
- `genericitems`: Classes permitting storage of data in atype-agnostic way in order to enable suitable [data management](overviews/dataManagement.md).
- `gui`: Qt-based GUI for `Dissolve`.
- `main`: Main class providing Dissolve's core object.
- `math`: Math-related classes, including classes containing (static) data operations and methods.
- `module`: Base definition and associated helper classes for defining and handling [modules](overviews/modules.md).
- `modules`: Repository containing subdirectores defining individual modules.
- `templates`: Template classes providing general, context-free functionality.
