# minimacore

MinimaCore is a C++ optimization library envisioned by Ariel Avi and Pedro Stella.

# How to contribute

- Code Coverage > 95%
- Maintain Code Style
    - If you use JetBrains IDEs, you can import the code style from [resources](resources/minimacore-code-style.xml)

# Getting Started

## Using the library

MinimaCore uses CMake as its build system and the main [CMakeLists.txt](CMakeLists.txt) defines several targets for
different algorithm types using aliases. You can link MinimaCore to your library
using `target_link_libraries(<YOUR_TARGET> minimacore::<OPTIMIZATION_ALGORITHM>)`.

Some algorithms may have pre-compiled binaries (i.e. for problems where the solution space is general enough). In those
cases, the targets are defined with a suffix that represents that (i.e. `*_double`). Most of the generic algorithms are
header-only and are more complicated to implement.

An overview of the algorithms available in this library is given below:

![](resources/algorithms_organogram.png)

## Dependencies

### C++20 capable compiler

### CMake

### Google Test

### Google Benchmark

### Eigen3