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

OpenHydroQual uses **jsoncpp** as a git submodule. You must pass `--recurse-submodules` when cloning, otherwise the `jsoncpp/` directory will be empty and the build will fail.

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

This produces `libOHQLib.so.2.0.4` (and symlinks `libOHQLib.so.2`, `libOHQLib.so`) in the build directory.

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

Download from https://cmake.org/download/ and add it to your PATH during installation.

Verify:

```cmd
cmake --version
```

### 5.4 Verify LAPACK/BLAS Libraries

The repository ships pre-built LAPACK and BLAS `.lib` files for Windows. Confirm they are present:

```
OpenHydroQual\libs\lapack-blas_lib_win64\
    release\
        lapack_win64_MT.lib
        blas_win64_MT.lib
    debug\
        lapack_win64_MTd.lib
        blas_win64_MTd.lib
```

If this directory is missing, download the pre-built libraries from
https://github.com/Reference-LAPACK/lapack/releases and place the `.lib` files
in the directory structure above.

### 5.5 Install Armadillo Headers

Download the latest Armadillo source from http://arma.sourceforge.net/download.html.
Extract it and copy the `include/` folder into the repository:

```
OpenHydroQual\include\
    armadillo
    armadillo_bits\
        ...
```

### 5.6 Open a Developer Command Prompt

From the Windows Start menu, search for **"Developer Command Prompt for VS 2022"** and open it.
This sets up the MSVC environment variables (`cl.exe`, `link.exe`, etc.).

Alternatively, from a regular `cmd.exe` or PowerShell, run:

```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

### 5.7 Configure with CMake

```cmd
cd OpenHydroQual\OHQLib
mkdir build
cd build

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DQt6_DIR="C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6" ^
    -DARMADILLO_INCLUDE_DIRS="%CD%\..\..\include" ^
    -DCMAKE_BUILD_TYPE=Release
```

> **Adjust paths** if your Qt installation is in a different location.
> Use `dir C:\Qt\` to find the exact version folder name.

You should see output ending with:

```
── OHQLib 2.0.4 ──────────────────────────────────
  Compiler     : MSVC 19.xx
  Build type   : Release
  Qt6Core      : 6.8.2 @ C:/Qt/6.8.2/msvc2022_64/lib/cmake/Qt6Core
  OpenMP       : TRUE
  Armadillo    : FALSE (header-only mode)
  LAPACK       : TRUE
  GSL          : FALSE
────────────────────────────────────────────────────
```

> GSL is Linux-only and will correctly show FALSE on Windows.

### 5.8 Build

```cmd
cmake --build . --config Release --parallel
```

This produces:
```
build\Release\OHQLib.dll      ← runtime library
build\Release\OHQLib.lib      ← import library (link against this)
```

### 5.9 (Optional) Build from Visual Studio IDE

After running `cmake ..` in step 5.7, a `OHQLib.sln` solution file is generated in the build directory. You can open this in Visual Studio and build from the IDE.

1. Open `build\OHQLib.sln` in Visual Studio
2. Set configuration to **Release** and platform to **x64**
3. Right-click **OHQLib** in Solution Explorer → **Build**

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

# All platforms — install pybind11
pip install pybind11
```

### 6.2 Linux / macOS — qmake Build (Current Method)

```bash
# Activate your virtual environment
source /path/to/your/venv/bin/activate

cd OpenHydroQual/PythonBindings
mkdir build && cd build

# Configure (adjust Qt version string to match your installation)
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

**Fix:** Pass `-DQt6_DIR` explicitly pointing to the Qt cmake directory:

```bash
# Linux (Qt installed via online installer)
cmake .. -DQt6_DIR=/opt/Qt/6.8.2/gcc_64/lib/cmake/Qt6

# macOS (Homebrew)
cmake .. -DQt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6

# Windows
cmake .. -DQt6_DIR="C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6"
```

---

### CMake cannot find Armadillo

```
CMake Error: Armadillo headers not found
```

**Fix:** Download Armadillo and pass the include path:

```bash
cmake .. -DARMADILLO_INCLUDE_DIRS=/path/to/armadillo/include
```

---

### jsoncpp source files missing

```
Error: cannot open source file "jsoncpp/src/lib_json/json_reader.cpp"
```

**Fix:** The submodule was not initialised. Run:

```bash
git submodule update --init --recursive
```

---

### OpenMP not found (macOS)

```
Warning: OpenMP not found — building without parallel support
```

**Fix:** Install libomp and help CMake find it:

```bash
brew install libomp
cmake .. -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include" \
         -DOpenMP_CXX_LIB_NAMES="omp" \
         -DOpenMP_omp_LIBRARY=/opt/homebrew/opt/libomp/lib/libomp.dylib
```

---

### Windows: `OHQLib.dll` not found at runtime

Python cannot find `OHQLib.dll` or `Qt6Core.dll` when importing the binding.

**Fix:** Add the directories containing the DLLs to your PATH before running Python:

```cmd
set PATH=C:\path\to\OHQLib\build\Release;C:\Qt\6.8.2\msvc2022_64\bin;%PATH%
python test_ohq.py
```

Or copy the DLLs alongside the `.pyd` binding file.

---

### MSVC: LNK2019 unresolved external symbols

Linker errors about missing symbols when building the Python bindings against OHQLib on Windows.

**Fix:** Ensure `WINDOWS_EXPORT_ALL_SYMBOLS ON` is set in `CMakeLists.txt` (it is, by default).
If the issue persists, rebuild OHQLib in the same configuration (Release/Debug) as the bindings.

---

### Wrong Python interpreter

The binding is compiled for a specific Python version. If you see:

```
ImportError: dynamic module does not define module export function
```

**Fix:** Ensure you are using the same Python version the binding was compiled against:

```bash
python3 --version
# Must match the cpython version tag in the .so filename
# e.g. openhydroqual_py.cpython-312-... requires Python 3.12
```
