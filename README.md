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
floating-point types) will be introduced in the future

Currently available targets:

`minimacore::genetic_algorithm` (incomplete)

## Build

minimacore uses [Conan](https://conan.io/) as the dependency manager. To make the dependencies available for CMake,
follow the steps given below

### Windows

To use dependencies in Debug and Release mode on MSVC, you have to build the dependencies packages for Debug and Release
separately:

```commandline
conan install . --build=missing -s build_type=Release
```

```commandline
conan install . --build=missing -s build_type=Debug
```

When calling CMake to compile with MSVC, make sure to use Visual Studio's generator and add the path
to `conan_toolchain.cmake` file.

```commandline
cmake .. -DCAMKE_TOOL_CHAIN_FILE=<path/to/root>/build/generators/conan_toolchain.cmake -G "Visual Studio 17 2022"
```

### Linux

Compilation on Linux is more straight forward.

```shell
conan install . --build=missing
```

Then assign the generated conan toolchain file to your CMake command:

```shell
cmake .. -DCAMKE_TOOL_CHAIN_FILE=<path/to/root>/build/generators/conan_toolchain.cmake
```

## Examples

The examples can be accessed in the [examples](examples) directory. Here is a list of available examples:

- Genetic Algorithm
    - [Rastrigin Function](examples/benchmark_functions/rastrigin/rastrigin.md)

## Dependencies

### C++20 capable compiler

This library uses [concepts](https://en.cppreference.com/w/cpp/language/constraints) (C++20), therefore, it is required
that your compiler is C++20-capable.

### CMake

This project is CMake based, therefore, users are encouraged to use CMake when using the library as well.

### Google Test

MinimaCore uses [Google Test](https://github.com/google/googletest) as unit testing framework. Sources for unit testing
can be found at [tests/src](tests/src).

### Google Benchmark

Together with Google Test, MinimaCore uses Google Benchmark as the micro benchmark library. This is only required to
compile and run smaller scripts that compare different approaches to the same problem.

### Eigen3

We decided to use Eigen3 as the matrix and vector operation calculations. Some reasons:

1. Eigen is open-source
2. Eigen is one of the [fastest and most efficient](https://eigen.tuxfamily.org/index.php?title=Benchmark) BLAS C++
   libraries
3. There are many
   [enterprise-level projects using Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page#Projects_using_Eigen)
4. It works with any floating point type

# How to contribute

Anyone is encouraged to contribute, that is with algorithms, CMake scripts or case studies.

- Please refer to and always use [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
- We target a code coverage of 100%, despite knowing that sometimes that is utopian.
- Code Style: code-style consistency is important to keep a sense of freshness in the library.
    - If you use JetBrains IDEs, you can import the code style from [resources](resources/minimacore-code-style.xml)