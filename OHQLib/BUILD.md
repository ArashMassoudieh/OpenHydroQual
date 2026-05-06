# Building OpenHydroQual

This guide covers every step to clone, configure, and build OpenHydroQual on Linux, macOS, and Windows. Follow the section for your platform.

---

## Table of Contents

1. [Clone the Repository](#1-clone-the-repository)
2. [Linux (Ubuntu/Debian) — CMake](#2-linux-ubuntudebian--cmake)
3. [Linux (Ubuntu/Debian) — QtCreator / qmake](#3-linux-ubuntudebian--qtcreator--qmake)
4. [macOS — CMake](#4-macos--cmake)
5. [Windows — CMake + MSVC](#5-windows--cmake--msvc)
6. [Building the Python Bindings](#6-building-the-python-bindings)
7. [Verifying the Build](#7-verifying-the-build)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Clone the Repository

OpenHydroQual uses **jsoncpp** as a git submodule. You must pass `--recurse-submodules`
when cloning, otherwise the `jsoncpp/` directory will be empty and the build will fail.

```bash
git clone --recurse-submodules https://github.com/ArashMassoudieh/OpenHydroQual.git
cd OpenHydroQual
```

If you already cloned without `--recurse-submodules`, initialise the submodule manually:

```bash
git submodule update --init --recursive
```

Confirm jsoncpp is present:

```bash
ls jsoncpp/src/lib_json/
# Should list: json_reader.cpp  json_value.cpp  json_writer.cpp
```

---

## 2. Linux (Ubuntu/Debian) — CMake

### 2.1 Install Dependencies

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    libarmadillo-dev \
    liblapack-dev \
    libblas-dev \
    libgsl-dev \
    libomp-dev
```

Verify the key tools:

```bash
cmake --version     # must be >= 3.20
g++ --version       # must be >= 9
qmake6 --version    # Qt 6.x
```

> **Qt version note:** If your distribution only ships Qt 5, install Qt 6 from the
> [Qt online installer](https://www.qt.io/download-qt-installer) and set
> `Qt6_DIR` as shown in step 2.3.

### 2.2 Navigate to the Library Directory

```bash
cd OpenHydroQual/OHQLib
```

### 2.3 Configure with CMake

**Standard (Qt6 found automatically):**

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**If Qt6 is installed in a non-standard location** (e.g. from the Qt online installer):

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DQt6_DIR=/opt/Qt/6.8.2/gcc_64/lib/cmake/Qt6
```

You should see a summary like:

```
── OHQLib 2.0.4 ──────────────────────────────────
  Compiler     : GNU 12.3.0
  Build type   : Release
  Qt6Core      : 6.8.2 @ /usr/lib/x86_64-linux-gnu/cmake/Qt6Core
  OpenMP       : TRUE
  Armadillo    : TRUE (12.6.4)
  LAPACK       : TRUE
  GSL          : TRUE
  Install dir  : /usr/local
────────────────────────────────────────────────────
```

### 2.4 Build

```bash
cmake --build . --parallel
```

This produces `libOHQLib.so.2.0.4` (and symlinks `libOHQLib.so.2`, `libOHQLib.so`) in the
build directory.

### 2.5 (Optional) Install System-Wide

```bash
sudo cmake --install .
sudo ldconfig
```

---

## 3. Linux (Ubuntu/Debian) — QtCreator / qmake

Use this path if you prefer to stay entirely within the Qt ecosystem.

### 3.1 Install Qt and QtCreator

Download and run the Qt Online Installer from https://www.qt.io/download-qt-installer.

During installation select:
- Qt 6.8.x → Desktop gcc 64-bit
- Qt Creator

### 3.2 Open the Project

1. Launch **Qt Creator**
2. Go to **File → Open File or Project**
3. Navigate to `OpenHydroQual/OHQLib/` and open `OHQLib.pro`
4. When prompted, select the **Desktop Qt 6.x.x GCC 64bit** kit
5. Click **Configure Project**

### 3.3 Build

1. Set the build configuration to **Release** (bottom-left kit selector)
2. Press **Ctrl+B** or go to **Build → Build All**

The library is output to:
```
OHQLib/build/Desktop_Qt_6_x_x-Release/libOHQLib.so.2.0.4
```

---

## 4. macOS — CMake

### 4.1 Install Dependencies

Install Xcode Command Line Tools if not already present:

```bash
xcode-select --install
```

Install Homebrew if not already present:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install required packages:

```bash
brew install cmake qt@6 armadillo libomp gsl
```

Add Qt to your PATH (add this to `~/.zshrc` or `~/.bash_profile`):

```bash
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
export Qt6_DIR="/opt/homebrew/opt/qt@6/lib/cmake/Qt6"
```

Reload your shell:

```bash
source ~/.zshrc
```

### 4.2 Configure and Build

```bash
cd OpenHydroQual/OHQLib
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

cmake --build . --parallel
```

This produces `libOHQLib.2.0.4.dylib` in the build directory.

> **Apple Silicon note:** Homebrew installs to `/opt/homebrew` on Apple Silicon (M1/M2/M3)
> and to `/usr/local` on Intel. The CMake file handles both. If CMake cannot find
> Armadillo, pass `-DARMADILLO_INCLUDE_DIRS=/opt/homebrew/include`.

---

## 5. Windows — CMake + MSVC

### 5.1 Install Visual Studio

Download and install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/) (free).

During installation, select the workload:
- **Desktop development with C++**

This installs MSVC, the Windows SDK, and CMake integration.

### 5.2 Install Qt for Windows

1. Download the [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. Run the installer and sign in (free Qt account required)
3. Under **Qt 6.8.x**, select:
   - **MSVC 2022 64-bit**
4. Complete the installation (default path: `C:\Qt\`)

### 5.3 Install CMake (Standalone)

If you prefer a standalone CMake rather than the one bundled with Visual Studio:

Download from https://cmake.org/download/ and select **"Add CMake to system PATH"**
during installation.

Verify:

```cmd
cmake --version   # must be >= 3.20
```

### 5.4 Verify LAPACK/BLAS Libraries

The repository ships pre-built LAPACK and BLAS `.lib` and `.dll` files for Windows.
Confirm they are present **directly** in the `lapack-blas_lib_win64` folder
(there are no `release/` or `debug/` subdirectories):

```
OpenHydroQual\libs\lapack-blas_lib_win64\
    lapack_win64_MT.lib
    blas_win64_MT.lib
    lapack_win64_MT.dll
    blas_win64_MT.dll
```

If this directory is missing, download the pre-built libraries from
https://github.com/Reference-LAPACK/lapack/releases and place the `.lib` and `.dll`
files directly in `libs\lapack-blas_lib_win64\`.

### 5.5 Install Armadillo Headers

Download the latest Armadillo source from http://arma.sourceforge.net/download.html.
Extract the archive. You only need the `include/` folder inside it.

Copy the contents of that `include/` folder into:

```
OpenHydroQual\include\
    armadillo
    armadillo_bits\
        ...
```

### 5.6 Install GSL

GSL is required on all platforms including Windows.

**Option A — vcpkg (recommended):**

Install vcpkg if not already present (substitute `C:\Projects` with wherever you
keep your development tools):

```cmd
cd C:\Projects
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

Install GSL:

```cmd
vcpkg install gsl:x64-windows
```

This produces the following files that are needed later:

```
C:\Projects\vcpkg\installed\x64-windows\
    include\gsl\        ← headers
    lib\gsl.lib
    lib\gslcblas.lib
    bin\gsl.dll
    bin\gslcblas.dll
```

**Option B — pre-built binaries:**

Download pre-built GSL for Windows from https://github.com/ampl/gsl/releases.
Extract it to a folder such as `C:\gsl\`. You will then set `GSL_ROOT_DIR` in
CMake GUI (see step 5.8).

### 5.7 Open CMake GUI

Launch **CMake (cmake-gui)** from the Start menu.

- **Where is the source code:** browse to `C:\Projects\OpenHydroQual\OHQLib`
- **Where to build the binaries:** browse to (or type) `C:\Projects\OpenHydroQual\OHQLib\build`

Click **Configure**. When asked, select:
- Generator: `Visual Studio 17 2022`
- Platform: `x64`

Click **Finish**. CMake will run and show errors — this is expected. Continue to the next step.

### 5.8 Set CMake Variables

After the first Configure run, set the following variables in the CMake GUI.
Use **Add Entry** for any that do not already appear in the list.

| Variable | Type | Value |
|---|---|---|
| `Qt6_DIR` | PATH | `C:\Qt\6.8.1\msvc2022_64\lib\cmake\Qt6` |
| `ARMADILLO_INCLUDE_DIRS` | PATH | `C:\Projects\OpenHydroQual\include` |

**If you used vcpkg for GSL**, add:

| Variable | Type | Value |
|---|---|---|
| `CMAKE_TOOLCHAIN_FILE` | FILEPATH | `C:\Projects\vcpkg\scripts\buildsystems\vcpkg.cmake` |

**If you used pre-built GSL binaries**, add:

| Variable | Type | Value |
|---|---|---|
| `GSL_ROOT_DIR` | PATH | `C:\gsl` (or wherever you extracted it) |

> **Tip:** Qt version numbers matter. Use File Explorer to confirm the exact
> version folder name under `C:\Qt\` and adjust `Qt6_DIR` accordingly.

### 5.9 Configure Again

Click **Configure** a second time. All errors should clear. You should see the
build summary at the bottom of the output window:

```
── OHQLib 2.0.4 ──────────────────────────────────
  Compiler     : MSVC 19.42.xxxxx
  Build type   :
  Qt6Core      : 6.8.1 @ C:/Qt/6.8.1/msvc2022_64/lib/cmake/Qt6Core
  OpenMP       : TRUE
  Armadillo    : FALSE (header-only mode)
  LAPACK       : TRUE
  GSL          : TRUE (C:/Projects/vcpkg/installed/x64-windows/lib/gsl.lib;...)
  Install dir  : C:/Program Files/OHQLib
────────────────────────────────────────────────────
```

### 5.10 Generate

Click **Generate**. This creates `OHQLib.sln` and related project files inside
the `build\` directory.

### 5.11 Build OHQLib

**Option A — from CMake GUI:**

Click **Open Project** to launch Visual Studio, then press **Ctrl+Shift+B** to
build all, or right-click **OHQLib** in Solution Explorer and select **Build**.

**Option B — from the command line:**

Open a **Developer Command Prompt for VS 2022** and run:

```cmd
cd C:\Projects\OpenHydroQual\OHQLib\build
cmake --build . --config Release --parallel
```

This produces:
```
build\Release\OHQLib.dll      ← runtime library
build\Release\OHQLib.lib      ← import library (link against this)
```

---

## 5a. Windows — Building OHQLibTest (Console Test Executable)

`OHQLibTest` is a standalone console application that loads and runs an `.ohq`
script file. It is useful for verifying that `OHQLib` is working correctly on
Windows without needing Python.

**Build `OHQLib` first (steps 5.1–5.11) before proceeding.**

### 5a.1 Create the OHQLibTest Directory

Create a folder `OHQLibTest\` at the same level as `OHQLib\` and place
`main.cpp` inside it:

```
OpenHydroQual\
    OHQLib\
    OHQLibTest\
        main.cpp
        CMakeLists.txt
```

### 5a.2 Create OHQLibTest\CMakeLists.txt

Create the file with the following contents. Adjust `GSL_DLL_DIR` and
`QT_DLL_DIR` to match your installation paths:

```cmake
cmake_minimum_required(VERSION 3.20)
project(OHQLibTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt6 ──────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core)

# ── OHQLib ───────────────────────────────────────────────────────
set(OHQLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OHQLib"
    CACHE PATH "Root of OHQLib source tree")
set(OHQLIB_BUILD_DIR "${OHQLIB_DIR}/build"
    CACHE PATH "OHQLib CMake build directory")

set(AQUIFOLIUM_ROOT "${OHQLIB_DIR}/../aquifolium")

add_library(OHQLib SHARED IMPORTED)
set_target_properties(OHQLib PROPERTIES
    IMPORTED_LOCATION_RELEASE        "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_LOCATION_DEBUG          "${OHQLIB_BUILD_DIR}/Debug/OHQLib.dll"
    IMPORTED_LOCATION_MINSIZEREL     "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_LOCATION_RELWITHDEBINFO "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll"
    IMPORTED_IMPLIB_RELEASE          "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
    IMPORTED_IMPLIB_DEBUG            "${OHQLIB_BUILD_DIR}/Debug/OHQLib.lib"
    IMPORTED_IMPLIB_MINSIZEREL       "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
    IMPORTED_IMPLIB_RELWITHDEBINFO   "${OHQLIB_BUILD_DIR}/Release/OHQLib.lib"
)

target_link_libraries(OHQLib INTERFACE Qt6::Core)

set(ARMADILLO_INCLUDE_DIRS "${OHQLIB_DIR}/../include"
    CACHE PATH "Path to Armadillo headers")
set(GSL_INCLUDE_DIRS ""
    CACHE PATH "Path to GSL headers (e.g. C:/Projects/vcpkg/installed/x64-windows/include)")

target_include_directories(OHQLib INTERFACE
    "${AQUIFOLIUM_ROOT}/include"
    "${AQUIFOLIUM_ROOT}/include/GA"
    "${AQUIFOLIUM_ROOT}/include/MCMC"
    "${AQUIFOLIUM_ROOT}/src"
    "${OHQLIB_DIR}/../"
    "${OHQLIB_DIR}/../jsoncpp/include"
    "${ARMADILLO_INCLUDE_DIRS}"
    "${GSL_INCLUDE_DIRS}"
)

# ── Executable ───────────────────────────────────────────────────
add_executable(OHQLibTest main.cpp)

target_compile_definitions(OHQLibTest PRIVATE
    GSL
    Terminal_version
    Q_JSON_SUPPORT
    windows_version
    ARMA_USE_LAPACK
    ARMA_USE_BLAS
)

target_link_libraries(OHQLibTest PRIVATE
    OHQLib
    Qt6::Core
)

# ── MSVC: copy DLLs and run windeployqt ──────────────────────────
if(MSVC)
    target_compile_options(OHQLibTest PRIVATE /W3 /utf-8)

    set(LAPACK_DLL_DIR "${OHQLIB_DIR}/../libs/lapack-blas_lib_win64")
    set(GSL_DLL_DIR    "C:/Projects/vcpkg/installed/x64-windows/bin"
        CACHE PATH "Directory containing gsl.dll and gslcblas.dll")
    set(QT_DLL_DIR     "C:/Qt/6.8.1/msvc2022_64/bin"
        CACHE PATH "Qt bin directory containing windeployqt.exe")
    set(OHQLIB_DLL     "${OHQLIB_BUILD_DIR}/Release/OHQLib.dll")

    add_custom_command(TARGET OHQLibTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OHQLIB_DLL}"
            "$<TARGET_FILE_DIR:OHQLibTest>/OHQLib.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LAPACK_DLL_DIR}/lapack_win64_MT.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/lapack_win64_MT.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LAPACK_DLL_DIR}/blas_win64_MT.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/blas_win64_MT.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GSL_DLL_DIR}/gsl.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/gsl.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GSL_DLL_DIR}/gslcblas.dll"
            "$<TARGET_FILE_DIR:OHQLibTest>/gslcblas.dll"
        COMMAND "${QT_DLL_DIR}/windeployqt.exe"
            "$<TARGET_FILE:OHQLibTest>"
        COMMENT "Copying runtime DLLs and running windeployqt"
    )
endif()
```

### 5a.3 Configure in CMake GUI

1. Open CMake GUI
2. Set **source** to `C:\Projects\OpenHydroQual\OHQLibTest`
3. Set **build** to `C:\Projects\OpenHydroQual\OHQLibTest\build`
4. Click **Configure** → `Visual Studio 17 2022` / `x64`
5. Set these variables:

| Variable | Type | Value |
|---|---|---|
| `Qt6_DIR` | PATH | `C:\Qt\6.8.1\msvc2022_64\lib\cmake\Qt6` |
| `OHQLIB_BUILD_DIR` | PATH | `C:\Projects\OpenHydroQual\OHQLib\build` |
| `ARMADILLO_INCLUDE_DIRS` | PATH | `C:\Projects\OpenHydroQual\include` |
| `GSL_INCLUDE_DIRS` | PATH | `C:\Projects\vcpkg\installed\x64-windows\include` |
| `GSL_DLL_DIR` | PATH | `C:\Projects\vcpkg\installed\x64-windows\bin` |
| `QT_DLL_DIR` | PATH | `C:\Qt\6.8.1\msvc2022_64\bin` |

6. Click **Configure** again — all errors should clear
7. Click **Generate**

### 5a.4 Build OHQLibTest

Click **Open Project** in CMake GUI, then in Visual Studio:

1. Right-click **OHQLibTest** in Solution Explorer → **Set as Startup Project**
2. Set configuration to **Release** and platform to **x64**
3. Press **Ctrl+Shift+B** to build

After a successful build, `build\Release\` will contain `OHQLibTest.exe` and all
required DLLs copied automatically by the post-build step:

```
OHQLibTest\build\Release\
    OHQLibTest.exe
    OHQLib.dll
    lapack_win64_MT.dll
    blas_win64_MT.dll
    gsl.dll
    gslcblas.dll
    Qt6Core.dll          ← and other Qt DLLs placed by windeployqt
    ...
```

### 5a.5 Run OHQLibTest from Visual Studio

1. Right-click **OHQLibTest** → **Properties**
2. Go to **Configuration Properties → Debugging**
3. Set **Command Arguments** to the full path of your `.ohq` script file:
   ```
   C:\Projects\OpenHydroQual\resources\your_model.ohq
   ```
4. Set **Working Directory** to the folder containing your model's input files:
   ```
   C:\path\to\your\model\folder\
   ```
5. If you see DLL-not-found errors at startup, add this to **Environment**:
   ```
   PATH=C:\Qt\6.8.1\msvc2022_64\bin;C:\Projects\vcpkg\installed\x64-windows\bin;%PATH%
   ```
6. Click **OK**, then press **F5** to run

Expected console output:

```
Input file: C:\...\your_model.ohq
Default Template path = ...
Executing script ...
Solving ...
Writing outputs in '...'
```

---

## 6. Building the Python Bindings

The Python bindings (`openhydroqual_py`) use **pybind11** and depend on `libOHQLib`.
Build `OHQLib` first (steps 2–5 above), then follow the instructions below.

### 6.1 Prerequisites

```bash
# Linux
sudo apt install python3-dev python3-pip python3-venv

# macOS
brew install python@3.12

# All platforms
pip install pybind11
```

### 6.2 Linux / macOS — qmake Build

```bash
# Activate your virtual environment
source /path/to/your/venv/bin/activate

cd OpenHydroQual/PythonBindings
mkdir build && cd build

qmake6 ../PythonBindings.pro CONFIG+=release
make -j$(nproc)
```

The output `.so` file (e.g. `openhydroqual_py.cpython-312-x86_64-linux-gnu.so`)
appears in the build directory.

### 6.3 Add the Binding to Your Python Path

Either copy the `.so` to your project directory, or add the build directory to
`sys.path` in your script:

```python
import sys
sys.path.append('/path/to/PythonBindings/build/Desktop_Qt_6_x_x-Release/')
import openhydroqual_py as ohq
```

Or install it into your virtual environment permanently:

```bash
cp openhydroqual_py*.so $(python3 -c "import site; print(site.getsitepackages()[0])")
```

---

## 7. Verifying the Build

Run the included test script to confirm everything works end-to-end:

```bash
cd OpenHydroQual/PythonBindings
python test_ohq.py
```

Expected output (abridged):

```
Running from time 0 to 90
Adjusting outputs ...
Simulation finished!
Done
```

If the simulation completes without errors, the build is successful.

---

## 8. Troubleshooting

### CMake cannot find Qt6

```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**Fix:** In CMake GUI, add an entry:
- Name: `Qt6_DIR`  Type: `PATH`
- Value: the `lib\cmake\Qt6` folder inside your Qt installation, e.g.:
  `C:\Qt\6.8.1\msvc2022_64\lib\cmake\Qt6`

The folder must contain a file called `Qt6Config.cmake`.

---

### IMPORTED_LOCATION not set for MinSizeRel / RelWithDebInfo

```
CMake Error: IMPORTED_LOCATION not set for imported target "lapack_win" configuration "MinSizeRel"
```

**Fix:** This means the `CMakeLists.txt` still has the old LAPACK block with separate
`release/` and `debug/` subdirectories. Replace it with the updated flat-path version
from this repository where all four configurations point to `lapack_win64_MT.lib`.

---

### Cannot open input file 'lapack_win64_MT.lib'

```
LINK : fatal error LNK1181: cannot open input file '...\lapack_win64_MT.lib'
```

**Fix:** The `.lib` files are expected directly in `libs\lapack-blas_lib_win64\`
with no subdirectories. Verify the layout:

```
OpenHydroQual\libs\lapack-blas_lib_win64\
    lapack_win64_MT.lib    ← must be here
    blas_win64_MT.lib      ← must be here
```

---

### GSL not found / ensureGSLInitialized identifier not found

```
CMake Error: GSL not found — it is required on all platforms
```
or
```
error C3861: 'ensureGSLInitialized': identifier not found
```

**Fix:** GSL must be installed and found by CMake on all platforms including Windows.

- **vcpkg:** `vcpkg install gsl:x64-windows` then set `CMAKE_TOOLCHAIN_FILE` in CMake GUI
- **Pre-built:** set `GSL_ROOT_DIR` in CMake GUI to the GSL installation folder
- **Linux:** `sudo apt install libgsl-dev`
- **macOS:** `brew install gsl`

---

### CMake cannot find Armadillo

```
CMake Error: Armadillo headers not found
```

**Fix:** Download Armadillo from http://arma.sourceforge.net, extract it, and set
`ARMADILLO_INCLUDE_DIRS` in CMake GUI to the `include/` subfolder of the extracted
archive, e.g. `C:\armadillo-12.8.0\include`.

---

### jsoncpp source files missing

```
Error: cannot open source file "jsoncpp/src/lib_json/json_reader.cpp"
```

**Fix:** The git submodule was not initialised. Run:

```bash
git submodule update --init --recursive
```

---

### OpenMP not found (macOS)

```
Warning: OpenMP not found — building without parallel support
```

**Fix:**

```bash
brew install libomp
cmake .. \
  -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include" \
  -DOpenMP_CXX_LIB_NAMES="omp" \
  -DOpenMP_omp_LIBRARY=/opt/homebrew/opt/libomp/lib/libomp.dylib
```

---

### Windows: OHQLib.dll or Qt6Core.dll not found at runtime

```
ImportError: DLL load failed while importing openhydroqual_py
```

**Fix:** Add the directories containing the required DLLs to your PATH before
running Python:

```cmd
set PATH=C:\Projects\OpenHydroQual\OHQLib\build\Release;^
         C:\Qt\6.8.1\msvc2022_64\bin;^
         C:\Projects\vcpkg\installed\x64-windows\bin;^
         %PATH%
python test_ohq.py
```

Or copy `OHQLib.dll`, `Qt6Core.dll`, and `gsl.dll` alongside the `.pyd` binding file.

---

### Windows: MSB3073 post-build copy command exited with code 1

```
Error MSB3073: The command "... cmake.exe -E copy_if_different ..." exited with code 1
```

This means one of the source DLL files in the post-build step does not exist at
the specified path. Check each path individually from a command prompt:

```cmd
dir C:\Projects\OpenHydroQual\OHQLib\build\Release\OHQLib.dll
dir C:\Projects\OpenHydroQual\libs\lapack-blas_lib_win64\lapack_win64_MT.dll
dir C:\Projects\OpenHydroQual\libs\lapack-blas_lib_win64\blas_win64_MT.dll
dir C:\Projects\vcpkg\installed\x64-windows\bin\gsl.dll
dir C:\Projects\vcpkg\installed\x64-windows\bin\gslcblas.dll
```

Common causes and fixes:

- **OHQLib.dll missing** — OHQLib has not been built yet, or was built to a different
  directory. Build OHQLib first and verify `OHQLIB_BUILD_DIR` in CMake GUI points to
  the correct build folder.
- **lapack_win64_MT.dll missing** — the repo may only contain `.lib` files. Remove
  the LAPACK/BLAS copy commands from the post-build step if no `.dll` files are present.
- **gsl.dll / gslcblas.dll missing** — vcpkg is installed in a non-default location.
  Set `GSL_DLL_DIR` in CMake GUI to the correct `bin\` folder, e.g.:
  `C:\Projects\vcpkg\installed\x64-windows\bin`
- **windeployqt.exe failing** — set `QT_DLL_DIR` in CMake GUI to match your Qt
  installation, e.g.: `C:\Qt\6.8.1\msvc2022_64\bin`

---

### Windows: Cannot open include file in OHQLibTest

The following headers are all required and come from different locations. If any
are missing, add the corresponding directory to `target_include_directories` in
`OHQLibTest\CMakeLists.txt`:

| Header | Source directory |
|---|---|
| `armadillo` | `OpenHydroQual\include\` |
| `TimeSeries.hpp` | `aquifolium\src\` |
| `json/json.h` | `jsoncpp\include\` |
| `QJsonArray` | Provided automatically via `Qt6::Core` |
| `gsl/gsl_math.h` | vcpkg: `C:\Projects\vcpkg\installed\x64-windows\include` |

---

### Windows: OHQLibTest — DLL not found when running from Visual Studio

If the executable launches but immediately crashes with a missing DLL error, add
the runtime directories to the Visual Studio debugging environment:

1. Right-click **OHQLibTest** → **Properties**
2. Go to **Configuration Properties → Debugging → Environment**
3. Add:
   ```
   PATH=C:\Qt\6.8.1\msvc2022_64\bin;C:\Projects\vcpkg\installed\x64-windows\bin;%PATH%
   ```

---

### Wrong Python interpreter

```
ImportError: dynamic module does not define module export function
```

**Fix:** The binding `.so`/`.pyd` is compiled for a specific Python version.
Ensure you are using the matching version:

```bash
python3 --version
# Must match the cpython tag in the filename
# e.g. openhydroqual_py.cpython-312-... requires Python 3.12
```

---

### MSVC: LNK2019 unresolved external symbols

**Fix:** Ensure `WINDOWS_EXPORT_ALL_SYMBOLS ON` is active in `CMakeLists.txt`
(it is by default). Also ensure OHQLib and the Python bindings are built with
the same configuration (both Release, or both Debug).
