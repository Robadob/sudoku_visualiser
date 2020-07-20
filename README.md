# Sudoku Visualiser

## Introduction

This project is a Sudoku client, built using C++ and OpenGL.

It has been built using the bare OpenGL engine found in [sdl_exp](https://github.com/Robadob/sdl_exp) and [FLAMEGPU2_visualiser](https://github.com/FLAMEGPU/FLAMEGPU2_visualiser).


### Dependencies

The dependencies below are required for building this project.

#### Required

* [CMake](https://cmake.org/) >= 3.12
* [git](https://git-scm.com/): Required by CMake for downloading dependencies
* *Linux:*
  * [make](https://www.gnu.org/software/make/)
  * gcc/g++ >= 6 (version requirements [here](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#system-requirements))
      * gcc/g++ >= 7 required for the test suite 
* *Windows:*
  * Visual Studio 2015 or higher (2019 preferred)
  
The below dependencies are automatically downloaded by CMake on Windows, they may required manually installing with your package manager on Linux. See CMake log for guidance.
  
* [SDL](https://www.libsdl.org/)
* [GLM](http://glm.g-truc.net/) *(consistent C++/GLSL vector maths functionality)*
* [GLEW](http://glew.sourceforge.net/) *(GL extension loader)*
* [FreeType](http://www.freetype.org/)  *(font loading)*

#### Optional
* [cpplint](https://github.com/cpplint/cpplint): Required for linting code

### Building 
This project uses [CMake](https://cmake.org/), as a cross-platform process, for configuring and generating build directives, e.g. `Makefile` or `.vcxproj`.

The project is primarily developed using Visual Studio 2019, so it may have minor issues if building with other compilers. However it is forked from projects which are maintained for both Windows and Linux, so should be easily fixed if required.

#### Windows

*Note: If installing CMake on Windows ensure CMake is added to the system path, allowing `cmake` to be used via `cmd`, this option is disabled within the installer by default.*

When generating Visual studio project files, using `cmake` (or `cmake-gui`), the platform **must** be specified as `x64`.

Using `cmake` this takes the form `-A x64`:

```
mkdir build && cd build
cmake .. -A x64
sudoku_visualiser.sln
```

This command will use the latest version of Visual studio detected, and open the created Visual studio solution.
If the `cmake` command fails, the detected version does not support CUDA. Reinstalling CUDA may correct this issue.

Alternatively using `-G` the desired version of Visual studio can be specified:

```
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015" -A x64
sudoku_visualiser.sln
```

`Visual Studio 14 2015` can be replaced with any supported [Visual studio generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) that is installed.

#### Linux

Under Linux, `cmake` can be used to generate makefiles specific to your system:

```
mkdir -p build && cd build
cmake .. 
make -j8
```

The option `-j8` enables parallel compilation using upto 8 threads, this is recommended to improve build times.

By default a `Makefile` for the `Release` build configuration will be generated.

Alternatively, using `-DCMAKE_BUILD_TYPE=Debug`, Debug build configurations can be generated:
 
```
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j8
```

#### Configuring CMake

There are currently no bespoke CMake options for this project.