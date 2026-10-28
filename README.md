# mcx_lib

## Running Tests

From the repository root in PowerShell, configure the top-level project, build
it, and run its tests:

```powershell
cmake -S . -B .\build\project-mingw -G "MinGW Makefiles"
cmake --build .\build\project-mingw
ctest --test-dir .\build\project-mingw --output-on-failure
```

The first command configures the project to use MinGW and writes the generated
build files to `build\project-mingw`. The second compiles the library and all
test executables. The third runs the tests registered with CTest;
`--output-on-failure` displays additional output for any test that fails. The
MinGW compiler and `mingw32-make` must be installed and available on `PATH`.
Use a new build directory if changing CMake generators or when reusing a
directory configured for a different source tree; CMake caches both settings.

## Building the Library and Mock App

The `mcx_lib` target builds the library. The separate `mcx_lib_mock` target
builds a mock application that links against the library:

```powershell
cmake --build .\build\project-mingw --target mcx_lib
cmake --build .\build\project-mingw --target mcx_lib_mock
```

The mock app exercises the public client lifecycle API:

```powershell
.\build\project-mingw\mcx_lib_mock.exe
```

## Installing the Public Library Package

The install package contains the compiled `mcx_lib`, the public
`mcx/client.hpp` header, and CMake package files. Internal headers and source
files are not installed. The library is built as a shared library: a `.dll`
and import library on Windows, or a `.so` on Linux. The consuming application
must be able to find the shared library at runtime. Configuration test
internals are built into a separate, non-installed support archive.

Install to a staging directory:

```powershell
cmake --install .\build\project-mingw --prefix .\package
```

A consumer project can then use the installed package:

```cmake
find_package(mcx_lib CONFIG REQUIRED)
target_link_libraries(my_gateway PRIVATE mcx::mcx_lib)
```

Configure the consumer with the install prefix available to CMake, for example:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\path\to\mcx_lib\package
```

To see output from passing tests, add `-V`:

```powershell
ctest --test-dir .\build\project-mingw -V
```

The default minimum log level is `INFO`. To include debug messages when
running the INI test directly, pass `--log-level DEBUG`:

```powershell
.\build\project-mingw\mcx_test_ini_config_test.exe --log-level DEBUG
```

Accepted levels are `DEBUG`, `INFO`, `WARNING`, `ERROR`, and `CRITICAL`.
CTest runs the registered test with the default `INFO` level; use the
executable command above to choose a level for an individual run.

## Extending the CMake Project

The top-level `CMakeLists.txt` defines the `mcx_lib` library from its
implementation files, installs only the supported `mcx/client.hpp` public
header, and defines the mock and test targets. New implementation files
(`.cpp`) placed anywhere under `src/` are
automatically added to the library, including files in new subdirectories.
Library headers belong under `include/`; only headers explicitly listed by
the install rules are part of the distributed API. For example, the installed
client header is included as `#include <mcx/client.hpp>`. Internal headers can
also live under `include/` for project organization without being installed.
No CMake source-list change is needed for new `.cpp` files under `src/`; CMake
detects them when the project is reconfigured.

Every `.cpp` file placed under `tests/` is automatically built as its own test
executable and registered with CTest. Use one test program (with its own
`main()`) per `.cpp` file; its relative path under `tests/` becomes the
executable and test name. No CMake changes are needed when adding test files.
Tests run with the repository root as their working directory, so they can
refer to project files such as `config.ini` by relative path. Test executables
link to `mcx_lib`, so its implementation files are compiled once and shared.

## Project Directory Layout
### Build
Contains all the output of building process

### Docs
Contains documentation files, explaining how the project works

### Include
Contains public header files .h / .hpp

### Libs
Contains external libraries

### Src
Contains .cpp files

### Tests
Contains automated test scripts