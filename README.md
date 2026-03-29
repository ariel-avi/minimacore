# minimacore

MinimaCore is an open-source modern C++ numerical optimization library (C++20 or later).

[![SonarCloud](https://sonarcloud.io/images/project_badges/sonarcloud-black.svg)](https://sonarcloud.io/summary/new_code?id=ariel-avi_minimacore)

[![Run Unit Tests](https://github.com/ariel-avi/minimacore/actions/workflows/tests.yml/badge.svg)](https://github.com/ariel-avi/minimacore/actions/workflows/tests.yml)
[![codecov](https://codecov.io/gh/ariel-avi/minimacore/graph/badge.svg?token=NGQBJPUWMB)](https://codecov.io/gh/ariel-avi/minimacore)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ariel-avi_minimacore&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=ariel-avi_minimacore)

# Background

After looking around for some open source libraries with low licensing restrictions, we couldn't find any that met our
requirements. The requirements are simple in terms of supported optimization problems and programming techniques.

- No convoluted software API
- Focus on continuous-domain problems
- Use of modern and modular build systems and APIs
- Easy to learn, use, implement and extend
- Leverage parallelism
- Open source and contribution-friendly

The library should also support different algorithm types that follow the same programming paradigms and conventions.
For example, if you need to solve a continuous-domain optimization problem, you should be able easily navigate through
different algorithm types to try different approaches for such problem. Changing from Genetic Algorithm to Differential
Evolution or Particle Swarm Optimization should be easy and intuitive enough that not much documentation is necessary to
migrate to a different solution.

# Authors

The library was conceptualized by Ariel Avi and Pedro Stella. We encourage any contribution from any source.

# Getting Started

## The library

We've conceptualized this library because we felt the lack of some open-source optimization libraries, especially in
modern C++.

An overview of the algorithms available in this library is given below*:

![](resources/algorithms_organogram.png)

(*) Not all algorithms stated above are implemented at the time of this writing

## Using the library

Targets in MinimaCore follow the naming convention:

`minimacore::<ALGORITHM_NAME>`

Replace `<ALGORITHM_NAME>` with the algorithm you wish to use (follow the naming convention in the `src/` folder). This
target is an alias to an interface target that includes the library's headers. Specialized targets (with defined
floating-point types) will be introduced in the future.

Currently available targets:

`minimacore::genetic_algorithm` (incomplete)

## Dependencies

### C++23 capable compiler

This library uses C++23 features, therefore, it is required that your compiler supports C++23.

### CMake >= 3.25

This project is CMake-based. CMake version 3.25 or later is required.

### Conan 2

MinimaCore uses [Conan](https://conan.io/) as the dependency manager. Install it via pip if you don't have it:

```shell
pip install conan
```

Conan manages the following dependencies:

- **Eigen3** — matrix and vector operations
- **Google Test** — unit testing framework
- **Google Benchmark** — micro-benchmarking library

## Build

### Step 1 — Install dependencies with Conan

#### Linux / macOS

Install dependencies for Release:

```shell
conan install . --build=missing -s build_type=Release
```

Or for Debug:

```shell
conan install . --build=missing -s build_type=Debug
```

#### Windows (MSVC)

On MSVC, install dependencies for both configurations separately:

```commandline
conan install . --build=missing -s build_type=Release
conan install . --build=missing -s build_type=Debug
```

### Step 2 — Configure and build with CMake

Conan generates CMake presets. Use them to configure and build:

#### Release

```shell
cmake --preset conan-release
cmake --build --preset conan-release
```

#### Debug

```shell
cmake --preset conan-debug
cmake --build --preset conan-debug
```

#### Windows (MSVC) — without presets

If presets are not available, pass the toolchain file manually:

```commandline
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Running the Tests

Tests are built automatically alongside the main library when Google Test is found. After building, run them using CTest:

#### Release

```shell
ctest --preset conan-release
```

#### Debug

```shell
ctest --preset conan-debug
```

Alternatively, run the test binary directly (from the build output directory):

```shell
./build/Release/minimacore_tests     # Release
./build/Debug/minimacore_tests       # Debug
```

## Examples

The examples can be accessed in the [examples](examples) directory. Here is a list of available examples:

- Genetic Algorithm
    - [Rastrigin Function](examples/benchmark_functions/rastrigin/rastrigin.md)

# How to contribute

Anyone is encouraged to contribute, that is with algorithms, CMake scripts or case studies.

- Please refer to and always use [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
- We target a code coverage of 100%, despite knowing that sometimes that is utopian.
- Code Style: code-style consistency is important to keep a sense of freshness in the library.
    - If you use JetBrains IDEs, you can import the code style from [resources](resources/minimacore-code-style.xml)