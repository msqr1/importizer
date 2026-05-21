This document provides a technical overview of the design and mechanism of importizer.

# Table of Contents

- [Prerequisite](#prerequisite)
- [Background](#background)

# Prerequisite

- This document contains [mermaid](https://mermaid.js.org) diagrams that requires a supported renderer such as Github Markdown.
- This document assumes:
  - You are comfortable with C++, C++ modules (check out [this excellent article](https://vector-of-bool.github.io/2019/03/10/modules-1.html) for a detailed introduction).
  - Read through the [README.md](README.md), especially the option section.
- Throughout the document you will see `[]` with an option name inside, it means replace it with the value of the option. For example, `[hdrExt]`, with default option will be replaced by `.hpp`.

# Background

I am a module advocate because they were brought in to address problems that comes with `#include`:

- **Slow Compilation:** Each `#include` recursively expands into many lines of code. For instance, `<iostream>` may add over 30,000 lines, and if used in multiple files, it’s processed repeatedly. Modules are compiled once and then reused.
- **Poor Encapsulation:** `#include` simply copies file content, exposing everything inside. Modules allow you to explicitly export only the necessary parts, keeping private details hidden.
- **Messy Syntax:** Modules eliminate the need for include guards or `#pragma once` because they are imported only once.

Because of that, I want to make modularization as easy as possible to encourage people to modularize their projects.
