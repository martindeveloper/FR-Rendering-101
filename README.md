# Flying Rat Rendering 101 Repository

Contains source code created during rendering course.

# Build

## Requirements

- Visual Studio C++ Compiler
- Windows SDK **10.0.22621.0** or newer
- CMake **3.5.0** or newer

## How To Build

### Using Command Line

```shell
# Configure
cmake -S . -B build/ -A x64

# Build
cmake --build build/ --config <Debug|Release> --parallel 4

# Build shaders only
cmake --build build/ --config <Debug|Release> --parallel 4 --target Shaders

# Clean
cmake --build build/ --target clean
```

### Using Visual Studio Code

In `.vscode/tasks.json` are prepared tasks for Visual Studio Code, in order to build, clean, etc. just open command palette and select **Tasks: Run Task** or **Tasks: Run Build Task**.

### Using Visual Studio

> [!IMPORTANT]
> Currently tested only with latest **2022** version.

- Make sure **C++ CMake tools for Windows** are installed using Visual Studio Installer
- Open Visual Studio
- Select **Open a local folder**
- Navigate to cloned repository folder and select it
- Visual Studio should detect CMake project by itself

## Run

Executable is emitted inside `build/<Configuration>/` directory, where `Configuration` could be `Debug` or `Release`.
